# Floating point routines
# using the Microsoft Floating Point format (5 bytes)
#
#  EEEEEEEE SAAAAAAA BBBBBBBB CCCCCCCC DDDDDDDD
#  =  (-1)^S * 2^(EEEEEEEE-128) * 0.1AAAAAAABBBBBBBBCCCCCCCCDDDDDDDD (base 2)
#  = 0 when EEEEEEEE is zero
#
# The floating point routines operate on a floating point accumulator
# FAC that occupies the same locations [0x81-0x87] as B0,B1,B2 and
# LAC. Register FAC is composed of a 8 bits exponent AE and a 40 bits
# mantissa AM whose high four bits overlap LAC. The routines
# internally use a second register FARG whose exponent BE and mantissa
# BM overlap the temporary registers T0, T1, and T2. Bit 7 of byte
# SIGN indicates whether FAC is negative. Bit 1 of SIGN indicates
# whether FAC and FARG have different signs. The exponent AE (resp BE)
# is 0 when FAC (resp FARG) is zero and and equal to EEEEEEEE otherwise.
#
# These routines therefore make use of all the area 0x81-0x8f.  Some
# of these calls might eventually use SYS calls in the future, meaning
# that none of the memory locations 'sysFn' and 'sysArgs[0-7]' should
# be assumed preserved.

def scope():

    T2L = T2
    T2H = T2+1
    T3L = T3
    T3H = T3+1

    AS = B0       # FACsign (bit7) FARGsign^FACsign (bit0).
    AE = B1       # FAC exponent
    AM = B2       # 40 bits FAC mantissa (one extra low byte)
    BM = T0       # 40 bits FARG mantissa (high byte overlaps T2L and CM)
    BE = T2H      # FARG exponent (overlaps T2H and CM)
    CM = T2       # extra 32 bits register
   
    # naming convention for exported symbols
    # '_@_xxxx' are the public api.  See their docstrings below.
    # '__@xxxx' are private and should not be relied upon.


    # ==== common things

    def code_fexception():
        '''All exceptions call _@_raise_ferr or _@_raise_fovf which are defined
           in the C library proper. If these functions return, what they leave in FAC 
           is returned by the API function (or more precisely by whathever 
           function called __@savevsp).'''
        nohop()
        label('__@fexception')   ### SIGFPE/exception
        _CALLJ('__@frestorevsp')
        _CALLJ('_@_raise_ferr')
        label('__@foverflow')    ### SIGFPE/overflow
        _CALLJ('__@frestorevsp')
        _LDI(0xffff);STW(AM+1);STW(AM+3);ST(AE);LDI(0);ST(AM)
        _CALLJ('_@_raise_fovf')

    def code_fsavevsp():
        nohop()
        label('__@fsavevsp')
        if args.cpu <= 5:
            LDWI('.vspfpe');STW(T2)
            LD(vSP);POKE(T2);RET()
        else:
            LDWI('.vspfpe');POKEA(vSP);RET()
        label('__@frestorevsp')
        label('.vspfpe',pc()+1)        
        LDI(0)  # this instruction is patched by fsavevsp.
        ST(vSP);RET()        
        
    module(name='rt_fexception.s',
           code=[ ('IMPORT', '_@_raise_ferr'),
                  ('IMPORT', '_@_raise_fovf'),
                  ('EXPORT', '__@fexception'),
                  ('EXPORT', '__@foverflow'),
                  ('EXPORT', '__@fsavevsp'),
                  ('EXPORT', '__@frestorevsp'),
                  ('CODE', '__@fexception', code_fexception),
                  ('CODE', '__@fsavevsp', code_fsavevsp) ] )

    def code_clrfac():
        '''_@_clrfac: -- Set FAC to zero
           _@_rndfac: -- Round FAC to 32 bit mantissa.'''
        nohop()
        label('_@_clrfac')
        LDI(0);STW(AE)        # [AE,AM]
        STW(AM+1);STW(AM+3)   # [AM+1,AM+2] [AM+3,AM+4]
        if args.cpu >= 6:
            LD(AS);ANDI(128);_BEQ('.ret')
            ORI(1);XORBA(AS);ST(AS)
        else:
            LD(AS);ANDI(128);_BEQ('.ret')
            ORI(1);XORW(AS);ST(AS)
        label('.ret')
        RET()
        ## Round fac
        label('_@_rndfac')
        LD(AE);_BEQ('_@_clrfac')
        LD(AM);ANDI(128);_BEQ('.rnd0')
        if args.cpu >= 6:
            INCL(AM+1);LDW(AM+1);ORW(AM+3);_BNE('.rnd0')
        else:
            LDI(1);ADDW(AM+1);STW(AM+1);_BNE('.rnd0')
            LDI(1);ADDW(AM+3);STW(AM+3);_BNE('.rnd0')
        LDI(128);ST(AM+4);INC(AE);LD(AE);_BNE('.rnd0')
        # overflow during rounding: just revert.
        _LDI(0xffff);STW(AM+1);STW(AM+3);ST(AE)
        label('.rnd0')
        LD(0);ST(AM)
        RET()

    module(name='rt_rndfac.s',
           code=[ ('EXPORT', '_@_rndfac'),
                  ('EXPORT', '_@_clrfac'),
                  ('CODE', '_@_clrfac', code_clrfac) ] )

    # ==== Load/store

    def load_mantissa(ptr, mantissa):
        # Load mantissa of float [ptr] into [mantissa,mantissa+3].
        # Returns high mantissa byte with sign bit.
        if args.cpu <= 5:
            LDI(4);ADDW(ptr);PEEK();ST(mantissa)
            LDI(3);ADDW(ptr);PEEK();ST(mantissa+1)
            LDI(2);ADDW(ptr);PEEK();ST(mantissa+2)
            LDI(1);ADDW(ptr);PEEK();ST(vACH)
            ORI(0x80);ST(mantissa+3);LD(vACH)
        else:
            LDW(ptr);INCW(vAC);
            PEEKAp(mantissa+3)
            PEEKAp(mantissa+2)
            PEEKAp(mantissa+1)
            PEEKA(mantissa)
            LD(mantissa+3)
            ORBI(0x80, mantissa+3)

    def code_fldfac():
        '''_@_fldfac: Load the float at address vAC into FAC: [vAC]->FAC'''
        nohop()
        label('_@_fldfac')
        STW(T3)
        label('__@fldfac_t3')
        LDW(T3);PEEK();ST(AE)
        load_mantissa(T3,AM+1)
        if args.cpu >= 6:
            XORBA(AS);ANDI(128);_BEQ('.fld1')
            XORBI(0x81, AS)
        else:
            XORW(AS);ANDI(128);_BEQ('.fld1')
            LD(AS);XORI(0x81);ST(AS)
        label('.fld1')
        LD(AE);_BNE('.fldr')
        STW(AM+1);STW(AM+3)
        label('.fldr')
        LDI(0);ST(AM)
        RET()

    module(name='rt_fldfac.s',
           code=[ ('EXPORT', '_@_fldfac'),
                  ('EXPORT', '__@fldfac_t3'),
                  ('CODE', '_@_fldfac', code_fldfac) ] )

    def code_fldarg():
        # [vAC]->FARG
        nohop()
        label('__@fldarg')
        STW(T3)
        label('__@fldarg_t3')
        LDW(T3);PEEK();ST(BE)
        load_mantissa(T3,BM+1)
        if args.cpu >= 6:
            XORBA(AS);ANDI(128);PEEK()
            XORBA(AS);ANDI(1);XORBA(AS);ST(AS)
        else:
            XORW(AS);ANDI(128);PEEK()
            XORW(AS);ANDI(1);XORW(AS);ST(AS)
        label('.fld1')
        LD(BE);_BNE('.fldr')
        STW(BM+1);STW(BM+3)
        label('.fldr')
        LDI(0);ST(BM)
        RET()

    module(name='rt_fldarg.s',
           code=[ ('EXPORT', '__@fldarg'),
                  ('EXPORT', '__@fldarg_t3'),
                  ('CODE', '__@fldarg', code_fldarg) ] )

    def code_fstfac():
        '''_@_fstfac: Store FAC at address vAC: FAC->[vAC]'''
        nohop()
        label('_@_fstfac')
        PUSH();STW(T3)
        LD(AE);POKE(T3);_BNE('.fst1')
        STW(AM+1);STW(AM+3)
        label('.fst1')
        LD(T3);SUBI(0xfc);_BGE('.slow')
        INC(T3);
        if args.cpu <= 5:
            LD(AS);ORI(0x7f);ANDW(AM+4);POKE(T3);INC(T3)
            LD(AM+3);POKE(T3);INC(T3)
            LD(AM+2);POKE(T3);INC(T3)
        else:
            LD(AS);ORI(0x7f);ANDW(AM+4);POKEp(T3)
            LD(AM+3);POKEp(T3)
            LD(AM+2);POKEp(T3)
        label('.fst2')
        LD(AM+1);POKE(T3)
        tryhop(2);POP();RET()
        label('.slow')
        if args.cpu >= 6:
            INCW(T3)
            LD(AS);ORI(0x7f);ANDW(AM+4);POKE(T3);INCW(T3)
            LD(AM+3);POKE(T3);INCW(T3)
            LD(AM+2);POKE(T3);INCW(T3)
            _BRA('.fst2')
        else:
            LDWI('.inc');CALL(vAC);
            LDWI('.poke');STW('sysArgs6')
            LD(AS);ORI(0x7f);ANDW(AM+4);CALL('sysArgs6')
            LD(AM+3);CALL('sysArgs6')
            LD(AM+2);CALL('sysArgs6')
            _BRA('.fst2')
            label('.poke')
            POKE(T3);
            label('.inc')
            LDI(1);ADDW(T3);STW(T3);RET()

    module(name = 'rt_fstfac.s',
           code = [ ('EXPORT', '_@_fstfac'),
                    ('IMPORT', '_@_clrfac'),
                    ('CODE', '_@_fstfac', code_fstfac) ] )

    def code_fac2farg():
        # Copy FAC to FARG
        nohop()
        label('__@fac2farg')
        LD(AE);ST(BE)
        LD(AM);ST(BM)
        LDW(AM+1);STW(BM+1)
        LDW(AM+3);STW(BM+3)
        if args.cpu >= 6:
            ANDBI(0xfe, AS)
        else:
            LD(AS);ANDI(0xfe);ST(AS)
        RET()

    module(name='rt_fac2farg.s',
           code=[ ('EXPORT', '__@fac2farg'),
                  ('CODE', '_@fac2farg', code_fac2farg) ] )

    # ==== shift left

    def macro_shl1(r, ext=True, ret=False):
        if ext:
            LDW(r+3);LSLW();LD(vACH);ST(r+4)
        lbl1 = genlabel()
        lbl2 = genlabel()
        LDW(r);_BLT(lbl1)
        LSLW();STW(r)
        LDW(r+2);LSLW();STW(r+2)
        RET() if ret else _BRA(lbl2)
        label(lbl1)
        LSLW();STW(r)
        LDW(r+2);LSLW();ORI(1);STW(r+2)
        RET() if ret else label(lbl2)

    def macro_shl4(r, ext=True):
        LDWI('SYS_LSLW4_46');STW('sysFn')
        if ext:
            LDW(r+3);SYS(46);LD(vACH);ST(r+4)
        LDW(r+2);SYS(46);LD(vACH);ST(r+3)
        LDW(r+1);SYS(46);LD(vACH);ST(r+2)
        LDW(r);SYS(46);STW(r)

    def macro_shl8(r, ext=True):
        if ext:
            LDW(r+2);STW(r+3)
        else:
            LD(r+2);ST(r+3)
        LDW(r);STW(r+1)
        LDI(0);ST(r)

    def macro_shl16(r, ext=True):
        if ext:
            LD(r+2);ST(r+4)
        LDW(r);STW(r+2)
        LDI(0);STW(r)

    def code_amshl1():
        nohop()
        label('__@amshl1')
        macro_shl1(AM, ext=True, ret=True)

    module(name='rt_fshl1.s',
           code=[ ('EXPORT', '__@amshl1'),
                  ('CODE', '__@amshl1', code_amshl1) ] )

    def code_amshl4():  # AM <<= 4
        nohop()
        label('__@amshl4')
        macro_shl4(AM, ext=True)
        RET()

    def code_amshl8():  # AM <<=8
        nohop()
        label('__@amshl8')
        macro_shl8(AM, ext=True)
        RET()

    def code_amshl16(): # AM <<= 16
        nohop()
        label('__@amshl16')
        macro_shl16(AM, ext=True)
        RET()

    module(name='rt_fshl.s',
           code=[ ('EXPORT', '__@amshl4'),
                  ('EXPORT', '__@amshl8'),
                  ('EXPORT', '__@amshl16'),
                  ('CODE', '__@amshl4', code_amshl4),
                  ('CODE', '__@amshl8', code_amshl8),
                  ('CODE', '__@amshl16', code_amshl16) ] )

    def code_bmshl1():
        nohop()
        label('__@bmshl1')
        macro_shl1(BM, ext=True, ret=True)

    module(name='rt_bmshl1.s',
           code=[ ('EXPORT', '__@bmshl1'),
                  ('CODE', '__@bmshl1', code_bmshl1) ] )

    def code_cmshl1():
        nohop()
        label('__@cmshl1')
        macro_shl1(CM, ext=False, ret=True)

    module(name='rt_cmshl1.s',
           code=[ ('EXPORT', '__@cmshl1'),
                  ('CODE', '__@cmshl1', code_cmshl1) ] )

    
    # ==== shift right

    def macro_shr16(r, ext=True):
        LDW(r+2);STW(r)
        if ext:
            LD(r+4);STW(r+2);LDI(0);ST(r+4)
        else:
            LDI(0);STW(r+2)

    def macro_shr8(r, ext=True):
        LDW(r+1);STW(r);
        if ext:
            LDW(r+3);STW(r+2);LDI(0);ST(r+4)
        else:
            LD(r+3);STW(r+2)

    def code_amshr16():
        nohop()
        label('__@amshr16')
        macro_shr16(AM, ext=True)
        RET()

    def code_amshr8():
        nohop()
        label('__@amshr8')
        macro_shr8(AM, ext=True)
        RET()

    def code_amshra():
        # shift am right by vAC positions
        label('__@amshra')
        PUSH();ALLOC(-2);STLW(0)
        ANDI(0xe0);_BEQ('.shra16')
        LD(AM+4);ST(AM)
        LDI(0);STW(AM+1);STW(AM+3)
        LDLW(0);ANDI(0xc0);_BEQ('.shra16')
        LDI(0);ST(AM);_BRA('.shraret')
        label('.shra16')
        LDLW(0);ANDI(16);_BEQ('.shra8')
        _CALLJ('__@amshr16')
        label('.shra8')
        LDLW(0);ANDI(8);_BEQ('.shra1to7')
        _CALLJ('__@amshr8')
        label('.shra1to7')
        LDLW(0);ANDI(7);_BEQ('.shraret')
        _CALLI('__@shrsysfn')
        LDW(AM);SYS(52);ST(AM)
        LDW(AM+1);SYS(52);ST(AM+1)
        LDW(AM+2);SYS(52);ST(AM+2)
        LDW(AM+3);SYS(52);STW(AM+3)
        label('.shraret')
        tryhop(2);ALLOC(2);POP();RET()

    module(name='rt_fshr.s',
           code=[ ('EXPORT', '__@amshra'),
                  ('IMPORT', '__@shrsysfn'),
                  ('EXPORT', '__@amshr16'),
                  ('EXPORT', '__@amshr8'),
                  ('CODE', '__@amshr16', code_amshr16),
                  ('CODE', '__@amshr8', code_amshr8),
                  ('CODE', '__@amshra', code_amshra) ]  )

    def code_bmshr8():
        nohop()
        label('__@bmshr8')
        macro_shr8(BM, ext=True)
        RET()

    module(name='rt_bmshr8.s',
           code=[ ('EXPORT', '__@bmshr8'),
                  ('CODE', '__@bmshr8', code_bmshr8) ]  )


    # ==== two complement

    def code_amneg():
        nohop()
        label('__@amneg')
        if args.cpu >= 6:
            NOTL(AM+1);XORBI(0xff,AM)
            INC(AM);LD(AM);_BNE('.ret')
            INCL(AM+1)
        else:
            LDI(0xff);XORW(AM+4);ST(AM+4)
            LDWI(0xffff);XORW(AM+2);STW(AM+2)
            LDWI(0xffff);XORW(AM);ADDI(1);STW(AM);BNE('.ret')
            LDI(1);ADDW(AM+2);STW(AM+2);BNE('.ret')
            INC(AM+4)
        label('.ret')
        RET()

    module(name='rt_amneg.s',
           code=[ ('EXPORT', '__@amneg'),
                  ('CODE', '__@amneg', code_amneg) ])


    # ==== normalization

    def code_fnorm():
        # Normalize FAC
        label('__@fnorm')
        PUSH()
        label('.norm1a')
        LDW(AM+3);_BLT('.done');_BNE('.norm1b')
        LD(AE);SUBI(16);_BLT('.norm1b');ST(AE)
        _CALLJ('__@amshl16')
        LDW(AM+3);_BNE('.norm1b')
        LDW(AM+1);_BNE('.norm1a')
        ST(AE);_BRA('.done')
        label('.norm1b')
        LD(AM+4);_BNE('.norm1c')
        LD(AE);SUBI(8);_BLT('.norm1c');ST(AE)
        _CALLJ('__@amshl8')
        label('.norm1c')
        LD(AM+4);ANDI(0xf0);_BNE('.norm1d')
        LD(AE);SUBI(4);_BLT('.norm1d');ST(AE)
        _CALLJ('__@amshl4')
        label('.norm1d')
        LDW(AM+3);_BLT('.done')
        LD(AE);SUBI(1);_BLT('.done');ST(AE)
        if args.cpu >= 6:
            NROL(5, AM)
        else:
            _CALLJ('__@amshl1')
        _BRA('.norm1d')
        label('.done')
        tryhop(2);POP();RET()

    module(name='rt_fnorm.s',
           code=[ ('EXPORT', '__@fnorm'),
                  ('IMPORT', '__@amshl16'),
                  ('IMPORT', '__@amshl8'),
                  ('IMPORT', '__@amshl4'),
                  ('IMPORT', '__@amshl1') if args.cpu < 6 else ('NOP',),
                  ('IMPORT', '__@foverflow'),
                  ('IMPORT', '_@_clrfac'),
                  ('CODE', '__@fnorm', code_fnorm) ] )
    
    # ==== conversions

    def code_fcv():
        '''_@_fcvu: Loads FAC with the unsigned long LAC.
           _@_fcvi: Loads FAC with the signed long LAC.'''
        label('_@_fcvu')
        PUSH()
        LDI(0);ST(AM);ST(AS);_BRA('.fcv1')
        label('_@_fcvi')
        PUSH()
        LDI(0);ST(AM)
        LD(AM+4);ANDI(128);STW(AS);_BEQ('.fcv1')
        _CALLJ('__@amneg')
        label('.fcv1')
        LDI(160);ST(AE)
        _CALLJ('__@fnorm')
        tryhop(2);POP();RET()

    module(name='rt_fcv.s',
           code=[ ('EXPORT', '_@_fcvi'),
                  ('EXPORT', '_@_fcvu'),
                  ('IMPORT', '__@fnorm'),
                  ('IMPORT', '__@amneg'),
                  ('CODE', '_@_fcvi', code_fcv) ] )

    def code_ftoi():
        '''_@_ftoi: Convert FAC into a signed long in LAC.
           _@_ftou: Convert FAC into an unsigned long in LAC.
           Both return 0x80000000 on overflow.'''
        label('_@_ftoi')
        PUSH()
        LD(AE);SUBI(160);_BLT('.ok')
        label('.ovf')
        _CALLJ('_@_clrfac')
        LDI(128);ST(LAC+3)
        tryhop(2);POP();RET()
        label('_@_ftou')
        PUSH()
        LD(AS);ANDI(128);BNE('.ovf')
        LD(AE);SUBI(160);_BGT('.ovf')
        label('.ok')
        XORI(255);ANDI(255);INC(vAC)
        _CALLI('__@amshra')
        LD(AS);ANDI(128);BEQ('.ret')
        _LNEG()
        label('.ret')
        tryhop(2);POP();RET()
        
    module(name='rt_fto.s',
           code=[ ('EXPORT', '_@_ftoi'),
                  ('EXPORT', '_@_ftou'),
                  ('IMPORT', '__@amshra'),
                  ('IMPORT', '_@_clrfac'),
                  ('CODE', '_@_ftoi', code_ftoi) ] )

    # ==== additions and subtractions

    def code_amaddbm():
        nohop()
        label('__@amaddbm')
        if args.cpu >= 6:
            LDI(BM+1);ADDLP() # four high bytes of AM/BM
            LD(BM);_BEQ('.a1')
            ADDBA(AM);ST(AM);LD(vACH);BEQ('.a1')
            INCL(AM+1);label('.a1')
        else:
            LD(AM);ADDW(BM);ST(AM);LD(vACH)
            BNE('.a1');LD(BM+1);BEQ('.a1');LDWI(0x100);label('.a1')
            ADDW(AM+1);ST(AM+1);LD(vACH)
            BNE('.a2');LD(AM+2);BEQ('.a2');LDWI(0x100);label('.a2')
            ADDW(BM+2);ST(AM+2);LD(vACH)
            BNE('.a3');LD(BM+3);BEQ('.a3');LDWI(0x100);label('.a3')
            ADDW(AM+3);ST(AM+3);LD(vACH)
            ADDW(BM+4);ST(AM+4)
        RET()

    module(name='rt_amaddbm.s',
           code=[ ('EXPORT', '__@amaddbm'),
                  ('CODE', '__@amaddbm', code_amaddbm) ])

    def code_amcarry():
        nohop()
        label('__@amcarry')
        if args.cpu >= 6:
            LDNI(1);NROR(5,AM)
        else:
            PUSH();LDI(1);_CALLI('__@amshra');POP()
            LD(AM+4);ORI(128);ST(AM+4)
        INC(AE);LD(AE);_BNE('.ret')
        _CALLJ('__@foverflow')
        label('.ret')
        RET()

    module(name='rt_amcarry.s',
           code=[ ('EXPORT', '__@amcarry'),
                  ('IMPORT', '__@foverflow'),
                  ('IMPORT', '__@amshra') if args.cpu < 6 else ('NOP',),
                  ('CODE', '__@amcarry', code_amcarry) ]  )

    def code_fadd_t3():
        label('__@fadd_t3')
        PUSH();_PEEKV(T3);STW(T2)
        LD(AE);_BEQ('.faddx1');SUBW(T2);_BGT('.faddx1')
        _CALLJ('__@fldarg_t3')          # FAC exponent <= arg exponent
        _BRA('.faddx2')
        label('.faddx1')
        _CALLJ('__@fac2farg')            # Otherwise
        _CALLJ('__@fldfac_t3')           # - ensure FAC exponent <= arg_exponent
        label('.faddx2')
        LD(AE);STW(T3)                   # - we don't need T3 anymore
        LD(BE);_BEQ('.faddx3')           # - adding zero is simple
        ST(AE)                           # - assume arg exponent
        SUBW(T3)
        _CALLI('__@amshra')              # - align fac mantissa
        LD(AS);ANDI(1);_BNE('.fsubx1')   # - are signs different?
        # addition branch
        LD(AM+4);XORI(128);ST(T3+1)
        if args.cpu >= 6:
            LDI(BM+1);ADDLP()
        else:
            _CALLJ('__@amaddbm')
        LDW(AM+3);ANDW(T3)
        _BLT('.faddx3')                  # > carry
        _CALLJ('__@amcarry')
        label('.faddx3')
        tryhop(2);POP();RET()
        # subtraction branch
        label('.fsubx1')
        LD(AS);XORI(0x80);ST(AS)         # - assume farg sign
        LD(AM+4);STW(T3+1)
        _CALLJ('__@amneg')               # - negate fac
        if args.cpu >= 6:                # - add
            LDI(BM+1);ADDLP()
        else:
            _CALLJ('__@amaddbm')
        LDW(AM+3);ANDW(T3)
        _BGE('.fsubx2')                  # > normalize
        LD(AS);XORI(129);ST(AS)          # - negate fac
        _CALLJ('__@amneg')
        label('.fsubx2')
        _CALLJ('__@fnorm')               # - normalize 
        tryhop(2);POP();RET()

    module(name='rt_faddt3.s',
           code=[ ('EXPORT', '__@fadd_t3'),
                  ('IMPORT', '__@amshra'),
                  ('IMPORT', '__@amneg'),
                  ('IMPORT', '__@fac2farg'),
                  ('IMPORT', '__@fldfac_t3'),
                  ('IMPORT', '__@fldarg_t3'),
                  ('IMPORT', '__@amaddbm') if args.cpu < 6 else ('NOP',),
                  ('IMPORT', '__@amcarry'),
                  ('IMPORT', '__@fnorm'),
                  ('CODE', '__@fadd_t3', code_fadd_t3) ] )

    def code_fadd():
        '''_@_fadd: Add the float at address vAC to FAC'''
        label('_@_fadd')
        PUSH();STW(T3)
        _CALLJ('__@fsavevsp')
        _CALLJ('__@fadd_t3')
        _CALLJ('_@_rndfac')
        tryhop(2);POP();RET()

    module(name='rt_fadd.s',
           code=[ ('EXPORT', '_@_fadd'),
                  ('CODE', '_@_fadd', code_fadd),
                  ('IMPORT', '__@fsavevsp'),
                  ('IMPORT', '__@fadd_t3'),
                  ('IMPORT', '_@_rndfac') ] )

    def code_fsub():
        '''_@_fsub: Subtract the float at address vAC from FAC'''
        label('_@_fsub')
        PUSH();STW(T3)
        _CALLJ('__@fsavevsp')
        if args.cpu >= 6:
            XORBI(0x81, AS)
        else:
            _CALLJ('_@_fneg')
        _CALLJ('__@fadd_t3')
        if args.cpu >= 6:
            XORBI(0x81, AS)
        else:
            _CALLJ('_@_fneg')
        _CALLJ('_@_rndfac')
        tryhop(2);POP();RET()

    module(name='rt_fsub.s',
           code=[ ('EXPORT', '_@_fsub'),
                  ('CODE', '_@_fsub', code_fsub),
                  ('IMPORT', '__@fsavevsp'),
                  ('IMPORT', '_@_fneg') if args.cpu < 6 else ('NOP',),
                  ('IMPORT', '__@fadd_t3'),
                  ('IMPORT', '_@_rndfac') ] )


    # ==== multiplication by 10

    def code_fmul10():
        '''_@_fmul10: Multiplies FAC by 10 with 38 bit mantissa precision.
           This function does not round its result to 32 bits.
           One should call _@_rndfac before storing its result.'''
        label('_@_fmul10')
        PUSH()
        _CALLJ('__@fsavevsp')
        LD(AE);ADDI(3);ST(AE);LD(vACH);_BNE('.err')
        _CALLJ('__@fac2farg')
        LDI(2);_CALLI('__@amshra')
        _CALLJ('__@amaddbm')
        LDW(AM+3);_BLT('.ret')
        _CALLJ('__@amcarry')
        label('.ret')
        tryhop(2);POP();RET()
        label('.err')
        _CALLJ('__@foverflow')

    module(name='rt_fmul10.s',
           code=[ ('EXPORT', '_@_fmul10'),
                  ('IMPORT', '__@amshra'),
                  ('IMPORT', '__@fac2farg'),
                  ('IMPORT', '__@amaddbm'),
                  ('IMPORT', '__@amcarry'),
                  ('IMPORT', '__@foverflow'),
                  ('CODE', '_@_fmul10', code_fmul10) ] )


    # ==== multiplication

    def code_macbm32x8():
        # macbm32x8: AM += (BM>>8) * T3L (trashes T3H)
        # smacbm32x8: Do AM>>=8 first.
        nohop()
        label('__@smacbm32x8')
        PUSH()
        _CALLJ('__@amshr8')
        _BRA('.prep')
        label('__@macbm32x8')
        PUSH()
        label('.prep')
        LD(T3L);_BEQ('.ret')
        _CALLJ('__@bmshr8')
        LDI(0);ST(BM+4)
        LDI(1)
        label('.loop')
        ST(T3H);ANDW(T3L);BEQ('.skip')
        _CALLJ('__@amaddbm')
        label('.skip')
        if args.cpu >= 6:
            LDI(0);NROL(5,BM)
        else:
            _CALLJ('__@bmshl1')
        LD(T3H);LSLW();LD(vAC);_BNE('.loop')
        label('.ret')
        POP();RET()

    module(name='rt_macbm32x8.s',
           code=[ ('EXPORT', '__@macbm32x8'),
                  ('EXPORT', '__@smacbm32x8'),
                  ('IMPORT', '__@amshr8'),
                  ('IMPORT', '__@bmshr8'),
                  ('IMPORT', '__@amaddbm'),
                  ('IMPORT', '__@bmshl1') if args.cpu < 6 else ('NOP',),
                  ('CODE', '__@macbm32x8', code_macbm32x8) ] )

    def code_fmulmac():
        label('__@fmulmac')
        PUSH();
        ALLOC(-4)
        LDW(AM+3);STLW(2)
        LDW(AM+1);STLW(0)
        LDI(0);ST(AM);STW(AM+1);STW(AM+3)
        LDLW(0);ST(T3L);_CALLJ('__@macbm32x8')
        LDLW(0);LD(vACH);ST(T3L);_CALLJ('__@smacbm32x8')
        LDLW(2);ST(T3L);_CALLJ('__@smacbm32x8')
        LDLW(2);LD(vACH);ST(T3L);_CALLJ('__@smacbm32x8')
        tryhop(4);ALLOC(4);POP();RET()

    def code_fmul():
        '''_@_fmul: Multiply FAC by the float at address vAC.'''
        label('_@_fmul')
        PUSH();STW(T3)
        _CALLJ('__@fsavevsp')
        _CALLJ('_@_rndfac')
        _CALLJ('__@fldarg_t3')
        LD(AS);ANDI(1);XORI(0xff);INC(vAC);ANDI(128);ST(AS) # sign
        LD(AE);_BEQ('.zero')
        LD(BE);_BEQ('.zero')
        ADDW(AE)                        # we know AM=0!
        SUBI(128+64);ALLOC(-2);STLW(0)  # result exponent on 16 bits (+ 64)
        _CALLJ('__@fmulmac')            # multiply!
        LDLW(0);ALLOC(2);STW(T3)        # T3 is now free
        LDI(64);ST(AE);
        _CALLJ('__@fnorm')              # normalize with dummy exponent
        LD(AE);ADDW(T3);_BLE('.zero')   # compute real exponent
        ST(AE);LD(vACH);_BNE('.ovf')    # check for overflow
        label('.fin')
        _CALLJ('_@_rndfac')             # round result
        tryhop(2);POP();RET()
        label('.ovf')
        _CALLJ('__@foverflow')
        label('.zero')
        _CALLJ('_@_clrfac')
        tryhop(2);POP();RET()
    
    module(name='rt_fmul.s',
           code=[ ('EXPORT', '_@_fmul'),
                  ('CODE', '_@_fmul', code_fmul),
                  ('IMPORT', '_@_rndfac'),
                  ('IMPORT', '__@fsavevsp'),
                  ('IMPORT', '__@fldarg_t3'),
                  ('IMPORT', '__@fnorm'),
                  ('IMPORT', '__@amshra'),
                  ('IMPORT', '__@foverflow'),
                  ('IMPORT', '_@_clrfac'),
                  ('IMPORT', '_@_rndfac'),
                  ('CODE', '__@fmulmac', code_fmulmac),
                  ('IMPORT', '__@macbm32x8'),
                  ('IMPORT', '__@smacbm32x8') ] )


    # ==== division

    def code_amsubbm32():
        # subtract BM<<8 from AM
        nohop()
        label('__@amsubbm32_')
        # alternating pattern
        LD(AM+1);SUBW(BM);ST(AM+1);LD(vACH)
        BNE('.a1');LD(BM+1);XORI(255);BEQ('.a1');LDWI(0x100);label('.a1')
        ADDW(AM+2);ST(AM+2);LD(vACH)
        BNE('.a2');LD(AM+3);BEQ('.a2');LDWI(0x100);label('.a2')
        SUBI(1);SUBW(BM+2);ST(AM+3);LD(vACH)
        BNE('.a3');LD(BM+3);XORI(255);BEQ('.a3');LDWI(0x100);label('.a3')
        ADDW(AM+4);ST(AM+4)
        RET()
        
    module(name='rt_amsubbm32.s',
           code=[ ('EXPORT', '__@amsubbm32_'),
                  ('CODE', '__@amsubbm32_', code_amsubbm32) ] )

    def code_fdivloop():
        '''LDLW(2) is the quotient exponent which is 
           reduced by one whenever one shifts one bit into CM. 
           This function repeats the division loop until 
           either the high bit is set or LDLW(2) reaches 0.'''
        label('__@fdivloop')
        PUSH()
        _CALLJ('__@bmshr8')              # - working with the low 32 bits of BM
        LDI(0);STW(CM);STW(CM+2)         #   makes room for CM
        _BRA('.fdl2')
        label('.fdl0')
        LDLW(2);SUBI(1);_BLT('.fdl5');STLW(2)
        if args.cpu >= 6:
            LDI(0);NROL(4,CM)
        else:
            _CALLJ('__@cmshl1')
        LDW(AM+3);_BGE('.fdl1')
        if args.cpu >= 6:
            LDI(0);NROL(5,AM)
        else:
            _CALLJ('__@amshl1')
        _BRA('.fdl3')
        label('.fdl1')
        if args.cpu >= 6:
            LDI(0);NROL(5,AM)
        else:
            _CALLJ('__@amshl1')
        label('.fdl2')
        if args.cpu >= 6:
            LDI(BM);CMPLPU();_BLT('.fdl4')
        else:
            LDW(AM+3);_CMPWU(BM+2);_BGT('.fdl3');_BLT('.fdl4')
            LDW(AM+1);_CMPWU(BM);_BLT('.fdl4')
        label('.fdl3')
        INC(CM)
        if args.cpu >= 6:
            LDI(BM);SUBLP()
        else:
            _CALLJ('__@amsubbm32_')
        label('.fdl4')
        LDW(CM+2);_BGE('.fdl0')
        label('.fdl5')
        tryhop(2);POP();RET()

    def code_fdivrnd():
        label('__@fdivrnd')
        PUSH()
        LDW(AM+3);_BLT('.fdr1')
        if args.cpu >= 6:
            NROL(5,AM)
        else:
            _CALLJ('__@amshl1')
        if args.cpu >= 6:
            LDI(BM);CMPLPU();_BLT('.fdr0')
        else:
            LDW(AM+3);_CMPWU(BM+2);_BGT('.fdr1');_BLT('.fdr0')
            LDW(AM+1);_CMPWU(BM);_BLT('.fdr0')
        label('.fdr1')
        LDI(1);ADDW(CM);STW(CM);_BNE('.fdr0')
        LDI(1);ADDW(CM+2);STW(CM+2);_BNE('.fdr0')
        _LDI(0xffff);STW(CM);STW(CM+2)
        label('.fdr0')
        tryhop(2);POP();RET()
        
    module(name='rt_fdivloop.s',
           code=[ ('EXPORT', '__@fdivloop'),
                  ('EXPORT', '__@fdivrnd'),
                  ('IMPORT', '__@amshl1') if args.cpu < 6 else ('NOP',),
                  ('IMPORT', '__@cmshl1') if args.cpu < 6 else ('NOP',),
                  ('IMPORT', '__@amshr8'),
                  ('IMPORT', '__@bmshr8'),
                  ('IMPORT', '__@amsubbm32_') if args.cpu < 6 else ('NOP',),
                  ('CODE', '__@fdivloop', code_fdivloop),
                  ('CODE', '__@fdivrnd', code_fdivrnd) ] )
        
    def code_fdiv():
        '''_@_fdiv: Divide FAC by the float at address vAC.'''
        label('_@_fdiv')
        PUSH();STW(T3)
        _CALLJ('__@fsavevsp')
        _CALLJ('_@_rndfac')
        _CALLJ('__@fldarg_t3')
        label('__@fdivfa')
        LD(AS);ANDI(1);XORI(0xff);INC(vAC);ANDI(128);ST(AS) # sign
        LD(BE);_BNE('.fdiv0')
        _CALLJ('__@fexception')          # - divisor is zero -> exception
        label('.fdiv0')
        LD(AE);_BEQ('.zero')             # - dividend is zero -> result is zero
        label('.fdiv1')
        LD(BE);STW(T3)
        LD(AE);SUBW(T3);ADDI(160)
        _BLE('.zero')                    # - hopeless underflow
        ALLOC(-2);STLW(0)
        _CALLJ('__@fdivloop')
        _CALLJ('__@fdivrnd')
        LDLW(0);ALLOC(2)
        ST(AE);_BLE('.zero')             # - copy exponent
        LD(vACH);_BNE('.ovf')            #   and test for overflow
        LDI(0);ST(AM)                    # - copy quotient into AM
        LDW(CM);STW(AM+1)
        LDW(CM+2);STW(AM+3)
        tryhop(2);POP();RET()
        label('.ovf')
        _CALLJ('__@foverflow')
        label('.zero')
        _CALLJ('_@_clrfac')
        tryhop(2);POP();RET()

    module(name='rt_fdiv.s',
           code=[ ('EXPORT', '_@_fdiv'),
                  ('EXPORT', '__@fdivfa'),
                  ('IMPORT', '__@fsavevsp'),
                  ('IMPORT', '_@_rndfac'),
                  ('IMPORT', '__@fldarg_t3'),
                  ('IMPORT', '__@fexception'),
                  ('IMPORT', '__@fdivloop'),
                  ('IMPORT', '__@fdivrnd'),
                  ('IMPORT', '_@_clrfac'),
                  ('IMPORT', '__@foverflow'),
                  ('CODE', '_@_fdiv', code_fdiv) ] )

    def code_fdivr():
        '''_@_fdiv: Divide the float at address vAC by FAC, result in FAC.'''
        label('_@_fdivr')
        label('_@_fdiv')
        PUSH();STW(T3)
        _CALLJ('__@fsavevsp')
        _CALLJ('_@_rndfac')
        _CALLJ('__@fac2farg')
        _CALLJ('__@fldfac_t3')
        _CALLJ('__@fdivfa') # no return
        
    module(name='rt_fdivr.s',
           code=[ ('EXPORT', '_@_fdivr'),
                  ('IMPORT', '__@fsavevsp'),
                  ('IMPORT', '_@_rndfac'),
                  ('IMPORT', '__@fac2farg'),
                  ('IMPORT', '__@fldfac_t3'),
                  ('IMPORT', '__@fdivfa'),
                  ('CODE', '_@_fdivr', code_fdivr) ] )
    
    # ==== fmod

    def code_fmod():
        '''_@_fmod: Leaves in FAC the floating point remainder FAC % [vAC]
           that is FAC - n * [vAC] where n is the quotient FAC/[vAC]
           rounded toward zero to an integer. Returns the 16 low bits
           of n into vAC'''
        label('_@_fmod')
        PUSH();STW(T3)
        _CALLJ('__@fsavevsp')
        _CALLJ('_@_rndfac')
        _CALLJ('__@fldarg_t3')
        LD(BE);_BEQ('.zero');STW(T3)
        LD(AE);_BEQ('.zero');SUBW(T3)
        _BLT('.zquo')
        ALLOC(-2);STLW(0)
        LD(BE);ST(AE)
        _CALLJ('__@fdivloop')
        LDLW(0);ALLOC(2);_BNE('.zero')
        _CALLJ('__@fnorm')
        label('.ret')
        LDW(CM)
        tryhop(2);POP();RET()
        label('.zero')
        _CALLJ('_@_clrfac')
        label('.zquo')
        LDI(0)
        tryhop(2);POP();RET()
        
    module(name='rt_fmod.s',
           code=[ ('EXPORT', '_@_fmod'),
                  ('CODE', '_@_fmod', code_fmod),
                  ('IMPORT', '__@fsavevsp'),
                  ('IMPORT', '_@_rndfac'),
                  ('IMPORT', '__@fldarg_t3'),
                  ('IMPORT', '__@fdivloop'),
                  ('IMPORT', '_@_clrfac'),
                  ('IMPORT', '__@fnorm') ] )

    # ==== comparisons

    def code_fcmp():
        '''_@_fcmp: Compare FAC with the float at address vAC and reeturn -1,
           0, or +1.  Note that because of the absence of subnormal
           numbers, this function might declare that two very small
           numbers are different even though subtracting one from the
           other might underflow and return zero.'''
        label('_@_fcmp')
        PUSH();STW(T3)
        _CALLJ('_@_rndfac')
        _CALLJ('__@fldarg_t3')
        LD(BE);_BNE('.fcmp0')
        LD(AE);_BEQ('.zero')
        label('.plus')
        LD(AS);XORI(128);ANDI(128);PEEK();LSLW();SUBI(1)
        tryhop(2);POP();RET()
        label('.minus')
        LD(AS);ANDI(128);PEEK();LSLW();SUBI(1)
        tryhop(2);POP();RET()
        label('.fcmp0')     # comparing sign
        LD(AS);ANDI(1);_BNE('.plus')
        label('.fcmp1')     # comparing exponents
        LD(BE);SUBW(AE)     # - [AE+1] = [AM] = 0 because of _@_rndfac above
        _BLT('.plus');_BGT('.minus')
        label('.fcmp2')     # comparing mantissa
        if args.cpu >= 6:
            LDI(BM+1);CMPLPU();_BLT('.minus');_BGT('.plus')
        else:
            LDW(AM+3);_CMPWU(BM+3);_BLT('.minus');_BGT('.plus')
            LDW(AM+1);_CMPWU(BM+1);_BLT('.minus');_BGT('.plus')
        label('.zero')
        LDI(0);tryhop(2);POP();RET()

    module(name='rt_fcmp.s',
           code=[ ('EXPORT', '_@_fcmp'),
                  ('IMPORT', '_@_rndfac'),
                  ('IMPORT', '__@fldarg_t3'),
                  ('CODE', '_@_fcmp', code_fcmp) ] )

    def code_fsign():
        '''_@_fsign: return the sign of FAC into AC (-1/0/+1)'''
        nohop()
        label('_@_fsign')
        PUSH()
        LD(AE);_BEQ('.done')
        LD(AS);ANDI(128);_BEQ('.plus')
        _LDI(-2)
        label('.plus')
        INC(vAC)
        label('.done')
        tryhop(2);RET()

    module(name='rt_fsign.s',
           code=[ ('EXPORT', '_@_fsign'),
                  ('IMPORT', '_@_rndfac'),
                  ('CODE', '_@_fsign', code_fsign) ] )

    # ==== misc

    def code_fneg():
        '''_@_fneg: Changes the sign of FAC. Fast'''
        nohop()
        label('_@_fneg')
        if args.cpu >= 6:
            XORBI(0x81, AS)
        else:
            LD(AS);XORI(0x81);ST(AS)
        RET()

    module(name='rt_fneg.s',
           code=[ ('EXPORT', '_@_fneg'),
                  ('CODE', '_@_fneg', code_fneg) ] )

    def code_frexp():
        '''_@_frexp: Return x in FAC and exp in vAC such
           that FAC = x * 2^exp with 0.5<=x<1.'''
        label('_@_frexp')
        PUSH()
        LD(AE);_BEQ('.frexp2')
        SUBI(128);STW(T3)
        LDI(128);ST(AE)
        LDW(T3)
        label('.frexp2')
        tryhop(2);POP();RET()
        
    module(name='rt_frexp.s',
           code=[ ('IMPORT', '__@fnorm'),
                  ('EXPORT', '_@_frexp'),
                  ('CODE', '_@_frexp', code_frexp) ] )
    
    def code_fscalb():
        '''_@_fscalb: Multiplies FAC by 2^vAC'''
        nohop()
        label('_@_fscalb')
        PUSH();STW(T3)
        _CALLJ('__@fsavevsp')
        LD(AE);ADDW(T3);BLE('.fscal1')
        ST(AE);LD(vACH);BEQ('.fscal0')
        _CALLJ('__@foverflow')
        label('.fscal1')
        _CALLJ('_@_clrfac')
        label('.fscal0')
        tryhop(2);POP();RET()

    module(name='rt_fscalb.s',
           code=[ ('IMPORT', '__@fsavevsp'),
                  ('IMPORT', '__@foverflow'),
                  ('IMPORT', '_@_clrfac'),
                  ('EXPORT', '_@_fscalb'),
                  ('CODE', '_@_fscalb', code_fscalb) ] )

    def code_fmask():
        label('__@fmask')
        PUSH();
        _CALLJ('__@fac2farg')
        LDWI(0xffff);STW(T3);ST(AM);STW(AM+1);STW(AM+3)
        LD(AE);SUBI(128);_BLE('.fmask1')
        _CALLI('__@amshra')
        label('.fmask1')
        tryhop(2);POP();RET()

    module(name='rt_fmask.s',
           code=[ ('IMPORT', '__@amshra'),
                  ('IMPORT', '__@fac2farg'),
                  ('EXPORT', '__@fmask'),
                  ('CODE', '__@fmask', code_fmask) ] )

    def code_frndz():
        '''_@_frndz: Make FAC, rounding it towards zero'''
        label('_@_frndz')
        PUSH()
        _CALLJ('__@fmask')
        LDW(T3);XORW(AM);ANDW(BM);ST(AM)
        LDW(T3);XORW(AM+1);ANDW(BM+1);STW(AM+1)
        LDW(T3);XORW(AM+3);ANDW(BM+3);STW(AM+3)
        _BNE('.frndz1')
        LDI(0);ST(AS);ST(AE)
        label('.frndz1')
        tryhop(2);POP();RET()

    module(name='rt_frndz.s',
           code=[ ('IMPORT', '__@fmask'),
                  ('EXPORT', '_@_frndz'),
                  ('CODE', '_@_frndz', code_frndz) ] )


# create all the modules
scope()

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
