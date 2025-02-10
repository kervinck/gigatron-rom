# Floating point routines
# using the Microsoft Floating Point format (5 bytes)
#
#  EEEEEEEE SAAAAAAA BBBBBBBB CCCCCCCC DDDDDDDD
#  =  (-1)^S * 2^(EEEEEEEE-128) * 0.1AAAAAAABBBBBBBBCCCCCCCCDDDDDDDD (base 2)
#  = 0 when EEEEEEEE is zero
#
# The floating point routines operate on a floating point accumulator
# FAC whose mantissa overlaps the extended and long accumulator LAC/LAC
# Register FAC is composed of a sign byte AS, an exponent byte AE and
# and a 40 bits mantissa AM. The routines internally use a second register
# FARG whose exponent BE and mantissa BM overlap T0/T1/sysArgs[0-4].
# Bit 7 of AS indicates whether FAC is negative. Bit 1 indicates
# whether FAC and FARG have different signs. The exponent AE (resp BE)
# is 0 when FAC (resp FARG) is zero and and equal to EEEEEEEE otherwise.

def scope():

    AS = FAS      # FAC sign (bit7) FARG sign (bit7^bit1)
    AE = FAE      # FAC exponent
    AM = LAX      # 40 bits FAC mantissa    (aka LAX)
    BM = T0       # 40 bits FARG mantissa   (aka sysArgs[0..4], T0-T1)
    BE = T2       # FARG exponent           (overlaps T2 CM)
    CM = T2       # extra 32 bits register  (overlaps T2-T3)

    # naming convention for exported symbols
    # '_@_xxxx' are the public api.  See their docstrings below.
    # '__@xxxx' are private and should not be relied upon.


    # ==== useful macros

    def _MOVQB(imm,d):
        if args.cpu >= 6:
            MOVQB(imm,d)
        else:
            LDI(imm);ST(d)

    avoid_cmpw = (args.cpu <= 4) or ("without_CmpOps" in rominfo)

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
        _LDI(0xffff);STW(AM+1);STW(AM+3);ST(AE);_MOVQB(0,AM)
        _CALLJ('_@_raise_fovf')

    def code_fsavevsp():
        nohop()
        label('__@fsavevsp')
        if args.cpu >= 6:
            LDWI('.vspfpe');DOKEA(vSP);RET()
        else:
            LDWI('.vspfpe');STW(T2)
            LDW(vSP);DOKE(T2);RET()
        label('__@frestorevsp')
        label('.vspfpe',pc()+1)
        LDWI(0)  # this instruction is patched by fsavevsp.
        if args.cpu >= 7:
            STW(vSP)
        else:
            ST(vSP)
        RET()

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
        LDW(AS-1);BGE('.ret')
        LD(AS);XORI(129);ST(AS)
        label('.ret')
        RET()
        ## Round fac
        label('_@_rndfac')
        LD(AE);_BEQ('_@_clrfac')
        LD(AM);ANDI(128);_BEQ('.rnd0')
        if args.cpu >= 6:
            INCVL(AM+1);LDW(AM+3);_BNE('.rnd0')
        else:
            LDI(1);ADDW(AM+1);STW(AM+1);_BNE('.rnd0')
            LDI(1);ADDW(AM+3);STW(AM+3);_BNE('.rnd0')
        _MOVQB(128,AM+4);INC(AE);LD(AE);_BNE('.rnd0')
        # overflow during rounding: just revert.
        _LDI(0xffff);STW(AM+1);STW(AM+3);ST(AE)
        label('.rnd0')
        _MOVQB(0,AM)
        RET()

    module(name='rt_rndfac.s',
           code=[ ('EXPORT', '_@_rndfac'),
                  ('EXPORT', '_@_clrfac'),
                  ('CODE', '_@_clrfac', code_clrfac) ] )

    # ==== Load/store

    def code_fldfac():
        '''_@_fldfac: Load the float at address vAC into FAC: [vAC]->FAC'''
        nohop()
        if args.cpu >= 7:
            label('__@fldfac_t3')
            LDW(T3)
            label('_@_fldfac')
            LDFAC()
            RET()
            warning("cpu7: use LDFAC instead of _@_fldfac")
        elif args.cpu >= 6:
            label('_@_fldfac')
            STW(T3)
            label('__@fldfac_t3')
            _PEEKV(T3);ST(AE);_BEQ('.fldz')
            INCV(T3);PEEKV(T3);ST(vACH);ORI(0x80);ST(AM+4)
            XORW(AS-1);_BGE('.fld1')
            LD(AS);XORI(0x81);ST(AS)
            label('.fld1')
            INCV(T3);PEEKV(T3);ST(AM+3)
            INCV(T3);PEEKV(T3);ST(AM+2)
            INCV(T3);PEEKV(T3);ST(AM+1)
            MOVQB(0,AM);RET()
            label('.fldz')
            ST(AM);STW(AM+1);STW(AM+2);RET()
        else:
            label('_@_fldfac')
            STW(T3)
            label('__@fldfac_t3')
            _PEEKV(T3);ST(AE);_BEQ('.fldz')
            LDI(4);ADDW(T3);PEEK();ST(AM+1)
            LDI(3);ADDW(T3);PEEK();ST(AM+2)
            LDI(2);ADDW(T3);PEEK();ST(AM+3)
            LDI(1);ADDW(T3);PEEK();ST(vACH);ORI(0x80);ST(AM+4)
            XORW(AS-1);_BGE('.fld1')
            LD(AS);XORI(0x81);ST(AS)
            label('.fld1')
            _MOVQB(0,AM);RET()
            label('.fldz')
            ST(AM);STW(AM+1);STW(AM+2);RET()

    module(name='rt_fldfac.s',
           code=[ ('EXPORT', '_@_fldfac'),
                  ('EXPORT', '__@fldfac_t3'),
                  ('CODE', '_@_fldfac', code_fldfac) ] )

    def code_fldarg():
        # [vAC]->FARG
        nohop()
        if args.cpu >= 7:
            label('__@fldarg_t3')
            LDW(T3)
            label('__@fldarg')
            LDFARG()
            MOVQB(0, BE+1)
            RET()
            warning("cpu7: use LDFARG instead of __@fldarg")
        elif args.cpu >= 6:
            label('__@fldarg')
            STW(T3)
            label('__@fldarg_t3')
            _PEEKV(T3);STW(BE);_BEQ('.fldz')
            INCV(T3);PEEKV(T3);ST(vACH);ORI(0x80);ST(BM+4)
            LD(vACH);XORW(AS);ANDI(128);PEEK()
            XORW(AS);ANDI(1);XORW(AS);ST(AS)
            INCV(T3);PEEKV(T3);ST(BM+3)
            INCV(T3);PEEKV(T3);ST(BM+2)
            INCV(T3);PEEKV(T3);ST(BM+1)
            MOVQB(0,BM);RET()
            label('.fldz')
            ST(BM);STW(BM+1);STW(BM+2);RET()
        else:
            label('__@fldarg')
            STW(T3)
            label('__@fldarg_t3')
            _PEEKV(T3);STW(BE);_BEQ('.fldz')
            LDI(4);ADDW(T3);PEEK();ST(BM+1)
            LDI(3);ADDW(T3);PEEK();ST(BM+2)
            LDI(2);ADDW(T3);PEEK();ST(BM+3)
            LDI(1);ADDW(T3);PEEK();ST(vACH);ORI(0x80);ST(BM+4)
            LD(vACH);XORW(AS);ANDI(128);PEEK()
            XORW(AS);ANDI(1);XORW(AS);ST(AS)
            _MOVQB(0,BM);RET()
            label('.fldz')
            ST(BM);STW(BM+1);STW(BM+2);RET()

    module(name='rt_fldarg.s',
           code=[ ('EXPORT', '__@fldarg'),
                  ('EXPORT', '__@fldarg_t3'),
                  ('CODE', '__@fldarg', code_fldarg) ] )

    def code_fstfac():
        '''_@_fstfac: Store FAC at address vAC: FAC->[vAC]'''
        nohop()
        label('_@_fstfac')
        if args.cpu >= 7:
            STFAC()
            warning("cpu7: use STFAC instead of _@_fstfac")
        elif args.cpu >= 6:
            STW(T3);LD(AE);POKE(T3);_BNE('.fst1')
            STW(AM+1);STW(AM+3)
            label('.fst1')
            LD(AS);ORI(0x7f);ANDW(AM+4);INCV(T3);POKE(T3)
            LD(AM+3);INCV(T3);POKE(T3)
            LD(AM+2);INCV(T3);POKE(T3)
            LD(AM+1);INCV(T3);POKE(T3)
            RET()
        else:
            STW(T3);LD(AE);POKE(T3);_BNE('.fst1')
            STW(AM+1);STW(AM+3)
            label('.fst1')
            LD(T3);SUBI(0xfc);_BGE('.slow');INC(T3);
            LD(AS);ORI(0x7f);ANDW(AM+4);POKE(T3);INC(T3)
            LD(AM+3);POKE(T3);INC(T3)
            LD(AM+2);POKE(T3);INC(T3)
            label('.fst2')
            LD(AM+1);POKE(T3)
            RET()
            label('.slow')
            PUSH()
            _CALLJ('.inc')
            if args.cpu < 5:
                LDWI('.poke');STW(T5)
            LD(AS);ORI(0x7f);ANDW(AM+4)
            if args.cpu < 5:
                CALL(T5)
            else:
                CALLI('.poke')
            LD(AM+3)
            if args.cpu < 5:
                CALL(T5)
            else:
                CALLI('.poke')
            LD(AM+2)
            if args.cpu < 5:
                CALL(T5)
            else:
                CALLI('.poke')
            POP();_BRA('.fst2')
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
        LD(AE);STW(BE)
        LD(AS);ANDI(0xfe);ST(AS)
        # Copy AM to BM
        label('__@am2bm40')
        if args.cpu >= 7:
            MOVF(AM,BM)   # works (miraculously)!
        else:
            LD(AM);ST(BM)
            LDW(AM+1);STW(BM+1)
            LDW(AM+3);STW(BM+3)
        RET()

    module(name='rt_fac2farg.s',
           code=[ ('EXPORT', '__@fac2farg'),
                  ('EXPORT', '__@am2bm40'),
                  ('CODE', '_@fac2farg', code_fac2farg) ] )

    # ==== shift left

    def code_amshl1():
        nohop()
        label('__@amshl1')
        if args.cpu >= 6:
            LSLVL(AM+1)
        else:
            LDW(AM+3);LSLW();STW(AM+3);
            LDW(AM+1);_BGE('.amshl1b');INC(AM+3)
            label('.amshl1b')
            LSLW();STW(AM+1)
        LD(AM);ADDW(AM);STW(AM)
        RET()

    module(name='rt_amshl1.s',
           code=[ ('EXPORT', '__@amshl1'),
                  ('CODE', '__@amshl1', code_amshl1) ] )

    def code_cmshl1():
        nohop()
        label('__@cmshl1')
        if args.cpu >= 6:
            warning('cpu6: use LSLVL instead of __@cmshl1')
            LSLVL(CM)
        else:
            LDW(CM+2);LSLW();STW(CM+2)
            LDW(CM);_BGE('.cmshl1b');INC(CM+2)
            label('.cmshl1b')
            LSLW();STW(CM)
        RET()

    module(name='rt_cmshl1.s',
           code=[ ('EXPORT', '__@cmshl1'),
                  ('CODE', '__@cmshl1', code_cmshl1) ] )

    def code_amshl8():  # AM <<=8
        nohop()
        label('__@amshl8')
        LDW(AM+2);STW(AM+3)
        LDW(AM);STW(AM+1)
        _MOVQB(0,AM)
        RET()

    module(name='rt_amshl8.s',
           code=[ ('EXPORT', '__@amshl8'),
                  ('CODE', '__@amshl8', code_amshl8) ] )

    # ==== shift right

    def code_amshr8():
        nohop()
        label('__@amshr8')
        LDW(AM+1);STW(AM);
        LDW(AM+3);STW(AM+2)
        _MOVQB(0,AM+4)
        RET()

    module(name='rt_amshr8.s',
           code=[ ('EXPORT', '__@amshr8'),
                  ('CODE', '__@amshr8', code_amshr8) ] )
 
    def code_amshra():
        # shift am right by vAC positions
        nohop()
        label('__@amshra')
        SUBI(40);_BLT('.shra1')
        LDI(0);ST(AM);STW(AM+1);STW(AM+3);RET()
        label('.shra1')
        if args.cpu >= 7:
            ADDI(40);LSRXA()
            RET()
        else:
            PUSH();
            ADDI(40-8);_BRA('.shra3')
            label('.shra2')
            _CALLJ('__@amshr8')
            LD(T5);SUBI(8)
            label('.shra3')
            ST(T5);_BGE('.shra2')
            if args.cpu < 5:
                LDWI('__@shrsysfn');STW('sysFn')
            LD(T5);ANDI(7);_BNE('.shra4')
            POP();RET()
            label('.shra4')
            if args.cpu < 5:
                CALL('sysFn')
            else:
                CALLI('__@shrsysfn')
            POP()
            label('__@amshrx')
            LDW(AM);SYS(52);ST(AM)
            LDW(AM+1);SYS(52);ST(AM+1)
            LDW(AM+2);SYS(52);ST(AM+2)
            LDW(AM+3);SYS(52);STW(AM+3)
            RET()

    module(name='rt_amshra.s',
           code=[ ('EXPORT', '__@amshra'),
                  ('EXPORT', '__@amshrx'),
                  ('IMPORT', '__@shrsysfn') if args.cpu < 7 else ('NOP',),
                  ('IMPORT', '__@amshr8') if args.cpu < 7 else ('NOP',),
                  ('CODE', '__@amshra', code_amshra) ] )


    # ==== two complement

    def code_amneg():
        nohop()
        label('__@amneg')
        if args.cpu >= 7:
            warning("cpu7: use NEGX instead of __@amneg")
            NEGX()
        else:
            LDI(0);SUBW(AM);STW(AM);_BNE('.amneg2')
            LDI(0);SUBW(AM+2);STW(AM+2);_BNE('.amneg4')
            LDI(0);SUBW(AM+4);ST(AM+4)
            RET()
            label('.amneg2')
            _LDI(0xffff);XORW(AM+2);STW(AM+2)
            label('.amneg4')
            LD(AM+4);XORI(0xff);ST(AM+4)
            RET()

    module(name='rt_amneg.s',
           code=[ ('EXPORT', '__@amneg'),
                  ('CODE', '__@amneg', code_amneg) ])


    # ==== normalization

    def code_fnorm():
        # Normalize FAC
        nohop()
        label('__@fnorm')
        LDW(AM+3);_BLT('.ret')
        if args.cpu >= 7:
            MOVQW(0,T3);_BNE('.norm1b')
            LDI(16);ADDV(T3);LDW(AM+1);_BNE('.norm1b')
            LDI(16);ADDV(T3);LD(AM);ST(vACH);_BNE('.norm1b')
            JEQ('_@_clrfac')
            label('.norm1a')
            LSLW();INC(T3)
            label('.norm1b')
            _BGE('.norm1a')
            LD(AE);SUBW(T3);JLT('_@_clrfac');ST(AE)
            LD(T3);LSLXA();
        else:
            PUSH();_BNE('.norm1')
            LD(AM);ORW(AM+1);_BEQ('.normz')
            label('.norm1')
            LD(AM+4);_BNE('.norm3')
            LD(AE);SUBI(8);_BLT('.normz');ST(AE)
            _CALLJ('__@amshl8')
            _BRA('.norm1')
            label('.norm2')
            LD(AE);SUBI(1);_BLT('.normz');ST(AE)
            _CALLJ('__@amshl1')
            label('.norm3')
            LDW(AM+3);_BGE('.norm2')
            tryhop(2);POP();RET()
            label('.normz')
            _CALLJ('_@_clrfac')
            tryhop(2);POP()
        label('.ret')
        RET()


    module(name='rt_fnorm.s',
           code=[ ('EXPORT', '__@fnorm'),
                  ('CODE', '__@fnorm', code_fnorm),
                  ('IMPORT', '_@_clrfac') ] +
                 ( [ ('IMPORT', '__@amshl8'),
                     ('IMPORT', '__@amshl1') ] if args.cpu < 7 else [] ) )


    # ==== conversions

    def code_fcv():
        '''_@_fcvu: Loads FAC with the unsigned long LAC.
           _@_fcvi: Loads FAC with the signed long LAC.'''
        nohop()
        label('_@_fcvu')
        PUSH()
        LDI(0);ST(AM);ST(AS);_BRA('.fcv1')
        label('_@_fcvi')
        PUSH()
        _MOVQB(0,AM)
        LD(AM+4);ANDI(128);STW(AS);_BEQ('.fcv1')
        if args.cpu >= 7:
            NEGVL(AM+1)
        else:
            _CALLJ('__@amneg')
        label('.fcv1')
        _MOVQB(160,AE)
        _CALLJ('__@fnorm')
        tryhop(2);POP();RET()

    module(name='rt_fcv.s',
           code=[ ('EXPORT', '_@_fcvi'),
                  ('EXPORT', '_@_fcvu'),
                  ('IMPORT', '__@fnorm'),
                  ('IMPORT', '__@amneg') if args.cpu < 7 else ('NOP',),
                  ('CODE', '_@_fcvi', code_fcv) ] )

    def code_ftoi():
        '''_@_ftoi: Convert FAC into a signed long in LAC.
           _@_ftou: Convert FAC into an unsigned long in LAC.
           Both return 0x80000000 on overflow.'''
        nohop()
        label('_@_ftoi')
        PUSH()
        LD(AE);SUBI(160);_BLT('.ok')
        label('.ovf')
        _CALLJ('_@_clrfac')
        LDI(128);ST(LAC+3)
        tryhop(2);POP();RET()
        label('_@_ftou')
        PUSH()
        LD(AS);ANDI(128);_BNE('.ovf')
        LD(AE);SUBI(160);_BGT('.ovf')
        label('.ok')
        if args.cpu >= 6:
            NEGV(vAC)
        else:
            XORI(255);ANDI(255);INC(vAC)
        _CALLI('__@amshra')
        LD(AS);ANDI(128);_BEQ('.ret')
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
        label('__@amaddbm40')
        LD(BM);ADDW(AM);ST(AM);XORW(AM);_BEQ('__@amaddbm32') # :-)
        if args.cpu >= 6:
            INCVL(LAC)
            label('__@amaddbm32')
            LDI(BM+1);ADDL()
            RET()
        else:
            LDI(1);ADDW(BM+1);BRA('.a0')
            label('__@amaddbm32')
            LDW(BM+1)
            label('.a0')
            PUSH();STW(vLR)
            ADDW(AM+1);STW(AM+1);_BLT('.a2')
            SUBW(vLR);ORW(BM+1);_BLT('.a3')
            label('.a1')
            LDW(BM+3);BRA('.a4')
            label('.a2')
            SUBW(vLR);ANDW(BM+1);_BGE('.a1')
            label('.a3')
            LDI(1);ADDW(BM+3)
            label('.a4')
            ADDW(AM+3);STW(AM+3)
            POP();RET()

    module(name='rt_amaddbm.s',
           code=[ ('EXPORT', '__@amaddbm32'),
                  ('EXPORT', '__@amaddbm40'),
                  ('CODE', '__@amaddbm', code_amaddbm) ])

    def code_fadd_t3_ss():
        nohop()
        # addition branch
        label('__@fadd_t3_ss')
        LD(AM+4);XORI(128);ST(T3+1)
        if args.cpu >= 6:
            LDI(BM+1);ADDL()
        else:
            _CALLJ('__@amaddbm32')
        LDW(AM+3);ANDW(T3)
        _BLT('.faddx1')
        # carry
        if args.cpu >= 7:
            LDI(1);RORX()
        elif args.cpu == 6:
            _LDI(0xffff);RORX(cpu6exact=False)
        else:
            LDWI('SYS_LSRW1_48');STW('sysFn');_CALLI('__@amshrx')
            LD(AM+4);ORI(128);ST(AM+4)
        INC(AE);LD(AE);_BNE('.faddx1')
        _CALLJ('__@foverflow')
        label('.faddx1')
        tryhop(2);POP();RET()

    def code_fadd_t3():
        label('__@fadd_t3')
        PUSH();_PEEKV(T3);STW(BE)
        LD(AE);_BEQ('.fadd1');SUBW(BE);_BGT('.fadd1')
        if args.cpu >= 7:
            LDW(T3);LDFARG()
        else:
            _CALLJ('__@fldarg_t3')       # FAC exponent <= arg exponent
        _BRA('.fadd2')
        label('.fadd1')
        _CALLJ('__@fac2farg')            # Otherwise swap
        if args.cpu >= 7:
            LDW(T3);LDFAC()
        else:
            _CALLJ('__@fldfac_t3')       # FAC exponent <= arg exponent
        label('.fadd2')
        LD(AE);STW(T3)                   # - we don't need T3 anymore
        LD(BE);_BEQ('.fadd3')            # - adding zero is simple
        ST(AE)                           # - assume arg exponent
        SUBW(T3)
        _CALLI('__@amshra')              # - align fac mantissa
        LD(AS);ANDI(1);_BNE('.fsubx1')   # - are signs different?
        _CALLJ('__@fadd_t3_ss')          # > same sign branch (no return)
        # different sign
        label('.fsubx1')
        if args.cpu >= 6:
            LD(AM+4);XORI(0x80);ST(T3+1)
            LDI(BM+1);SUBL()             # - subtract
            LDW(AM+3);ORW(T3)
        else:
            LD(AS);XORI(0x80);ST(AS)     # - assume farg sign
            LD(AM+4);ST(T3+1)
            _CALLJ('__@amneg')           # - negate fac
            _CALLJ('__@amaddbm32')       # - add
            LDW(AM+3);ANDW(T3)
        _BGE('.fsubx2')                  # > normalize
        LD(AS);XORI(129);ST(AS)          # - negate fac
        if args.cpu >= 7:
            NEGX()
        else:
            _CALLJ('__@amneg')
        label('.fsubx2')
        _CALLJ('__@fnorm')               # - normalize
        label('.fadd3')
        tryhop(2);POP();RET()

    module(name='rt_faddt3.s',
           code=[ ('EXPORT', '__@fadd_t3'),
                  ('IMPORT', '__@foverflow'),
                  ('IMPORT', '__@amshra'),
                  ('IMPORT', '__@amneg') if args.cpu < 7 else ('NOP',),
                  ('IMPORT', '__@fac2farg'),
                  ('IMPORT', '__@fldfac_t3') if args.cpu < 7 else ('NOP',),
                  ('IMPORT', '__@fldarg_t3') if args.cpu < 7 else ('NOP',),
                  ('IMPORT', '__@amaddbm32') if args.cpu < 6 else ('NOP',),
                  ('IMPORT', '__@fnorm'),
                  ('CODE', '__@fadd_t3_ss', code_fadd_t3_ss),
                  ('CODE', '__@fadd_t3', code_fadd_t3) ] )

    def code_fadd():
        '''_@_fadd: Add the float at address vAC to FAC'''
        nohop()
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
        nohop()
        label('_@_fsub')
        PUSH();STW(T3)
        _CALLJ('__@fsavevsp')
        LD(AS);XORI(0x81);ST(AS)
        _CALLJ('__@fadd_t3')
        LD(AS);XORI(0x81);ST(AS)
        _CALLJ('_@_rndfac')
        tryhop(2);POP();RET()

    module(name='rt_fsub.s',
           code=[ ('EXPORT', '_@_fsub'),
                  ('CODE', '_@_fsub', code_fsub),
                  ('IMPORT', '__@fsavevsp'),
                  ('IMPORT', '_@_fneg'),
                  ('IMPORT', '__@fadd_t3'),
                  ('IMPORT', '_@_rndfac') ] )


    # ==== multiplication

    def code_macx():
        '''Emulation of DEV7ROM's MACX opcode.
           * Add vACL (8 bits) times sysArgs[0..3] (32 bits) to LAX (40 bits)
           * LAC := LAX + sysArgs[3..0] * vACL.
           * Trashes sysArgs[4-7] like MACX'''
        nohop()
        if args.cpu >= 7:
            warning("cpu7: use MACX instead of __@macx")
            label('__@macx_t5')
            LD(T5)
            label('__@macx')
            MACX();RET()
        else:
            label('__@macx')
            ST(T5)
            label('__@macx_t5')
            PUSH()
            _MOVQB(0,BM+4)  # BM+4 is T4L
            LDI(1);ST(T4+1) # Use T4H for mask
            label('.macx1')
            ANDW(T5);_BEQ('.macx2')
            _CALLJ('__@amaddbm40')
            label('.macx2')
            if args.cpu >= 6:
                LSLVL(BM+1)
            else:
                LDW(BM+1);_BLT('.sh1')
                LSLW();STW(BM+1)
                LDW(BM+3);LSLW();_BRA('.sh2')
                label('.sh1')
                LSLW();STW(BM+1)
                LDW(BM+3);LSLW();ORI(1)
                label('.sh2')
                STW(BM+3)
            LD(BM);ADDW(BM);STW(BM)
            LD(T4+1);LSLW();ST(T4+1)
            LD(vACL);_BNE('.macx1')
            LDW(BM+1);STW(BM)
            LDW(BM+3);STW(BM+2)
            POP();RET()

    module(name='rt_macx.s',
           code=[ ('EXPORT', '__@macx'),
                  ('EXPORT', '__@macx_t5'),
                  ('IMPORT', '__@amaddbm40') if args.cpu < 7 else ('NOP',),
                  ('CODE', '__@macx', code_macx) ] )

    def code_fmulm():
        nohop()
        label('__@fmulm')
        if args.cpu < 7:
            PUSH();
        ALLOC(-2)
        _MOVW(BM+1, BM)
        _MOVW(BM+3, BM+2)
        _MOVW(AM+3, T3)
        LDW(AM+1);STLW(0)
        if args.cpu >= 6:
            MOVQB(0,AM);MOVQW(0,AM+1);MOVQW(0,AM+3)
        else:
            LDI(0);ST(AM);STW(AM+1);STW(AM+3);LDLW(0)
        if args.cpu >= 7:
            MACX();LDI(8);LSRXA()
        else:
            ST(T5);_CALLJ('__@macx_t5');_CALLJ('__@amshr8')
        LDLW(0);LD(vACH)
        if args.cpu >= 7:
            MACX();LDI(8);LSRXA()
        else:
            ST(T5);_CALLJ('__@macx_t5');_CALLJ('__@amshr8')
        LD(T3)
        if args.cpu >= 7:
            MACX();LDI(8);LSRXA()
        else:
            ST(T5);_CALLJ('__@macx_t5');_CALLJ('__@amshr8')
        LD(T3+1)
        if args.cpu >= 7:
            MACX()
        else:
            ST(T5);_CALLJ('__@macx_t5')
        ALLOC(2)
        if args.cpu < 7:
            tryhop(2);POP()
        RET()

    def code_fmul():
        '''_@_fmul: Multiply FAC by the float at address vAC.'''
        nohop()
        label('_@_fmul')
        PUSH();STW(T3)
        _CALLJ('__@fsavevsp')
        if args.cpu >= 7:
            LDW(T3);LDFARG()
        else:
            _CALLJ('__@fldarg_t3')
        LD(AS);ANDI(1);ADDI(127);ST(AS) # sign
        LD(BE);STW(BE);_BEQ('.zero')
        LD(AE);_BEQ('.zero')
        if args.cpu >= 7:
            ADDV(BE)                    # result exponent (+128)
        else:
            ADDW(BE);STW(BE)
        _CALLJ('__@fmulm')              # multiply!
        LDI(64);ST(AE);
        _CALLJ('__@fnorm')              # normalize with dummy exponent
        LD(AE);ADDW(BE);SUBI(128+64)    # real exponent on 16 bits
        _BLE('.zero')                   # -> underflow
        ST(AE);LD(vACH);_BNE('.ovf')    # -> overflow
        label('.fin')
        _CALLJ('_@_rndfac')             # round
        tryhop(2);POP();RET()
        label('.ovf')
        _CALLJ('__@foverflow')
        label('.zero')
        _CALLJ('_@_clrfac')
        tryhop(2);POP();RET()

    module(name='rt_fmul.s',
           code=[ ('EXPORT', '_@_fmul'),
                  ('IMPORT', '__@fsavevsp'),
                  ('IMPORT', '__@fldarg_t3') if args.cpu < 7 else ('NOP',),
                  ('IMPORT', '__@fnorm'),
                  ('IMPORT', '__@amshra'),
                  ('IMPORT', '__@foverflow'),
                  ('IMPORT', '_@_clrfac'),
                  ('IMPORT', '_@_rndfac'),
                  ('IMPORT', '__@macx_t5') if args.cpu < 7 else ('NOP',),
                  ('CODE', '__@fmulm', code_fmulm),
                  ('CODE', '_@_fmul', code_fmul) ] )

    # ==== division

    def code_fdivloop():
        '''[vSP] is the quotient exponent which is reduced by one whenever one
           shifts one bit into CM.  This function repeats the division
           loop until either the high bit is set or [vSP] reaches 0.'''
        nohop()
        label('__@fdivloop')
        qexp = 0              # - stack offset of quotient exponent
        if args.cpu < 7 or avoid_cmpw:
            qexp = 2; PUSH()
        _MOVW(BM+1, BM);      # - working with the low 32 bits of BM
        _MOVW(BM+3, BM+2)
        LDI(0);STW(CM);STW(CM+2)
        _BRA('.fdl2')
        label('.fdl0')
        LDLW(qexp);SUBI(1);_BLT('.fdl5');STLW(qexp)
        if args.cpu >= 6:
            LSLVL(CM)
        else:
            _CALLJ('__@cmshl1')
        LDW(AM+3);_BGE('.fdl1')
        if args.cpu >= 7:
            LSLVL(AM+1)
        else:
            _CALLJ('__@amshl1')
        _BRA('.fdl3')
        label('.fdl1')
        if args.cpu >= 7:
            LSLVL(AM+1)
        else:
            _CALLJ('__@amshl1')
        label('.fdl2')
        if args.cpu >= 6:
            LDI(BM);CMPLU()
        elif avoid_cmpw:
            _CALLJ('__@lcmpu_t0t1')
        else:
            LDW(AM+3);_CMPWU(BM+2);_BNE('.fdl2a')
            LDW(AM+1);_CMPWU(BM);label('.fdl2a')
        _BLT('.fdl4')
        label('.fdl3')
        INC(CM)
        if args.cpu >= 6:
            LDI(BM);SUBL()
        else:
            _CALLJ('__@lsub_t0t1')
        label('.fdl4')
        LDW(CM+2);_BGE('.fdl0')
        label('.fdl5')
        if args.cpu < 7 or avoid_cmpw:
            tryhop(2);POP()
        RET()

    def code_fdivrnd():
        nohop()
        label('__@fdivrnd')
        if args.cpu < 7 or avoid_cmpw:
            PUSH()
        LDW(AM+3);_BLT('.fdr1')
        if args.cpu >= 7:
            LSLVL(AM+1)
        else:
            _CALLJ('__@amshl1')
        if args.cpu >= 6:
            LDI(BM);CMPLU()
        elif avoid_cmpw:
            _CALLJ('__@lcmpu_t0t1')
        else:
            LDW(AM+3);_CMPWU(BM+2);_BNE('.fdr2')
            LDW(AM+1);_CMPWU(BM);label('.fdr2')
        _BLT('.fdr0')
        label('.fdr1')
        if args.cpu >= 7:
            INCVL(CM);LDW(CM);ORW(CM+2);_BNE('.fdr0')
        else:
            LDI(1);ADDW(CM);STW(CM);_BNE('.fdr0')
            LDI(1);ADDW(CM+2);STW(CM+2);_BNE('.fdr0')
        _LDI(0xffff);STW(CM);STW(CM+2)
        label('.fdr0')
        if args.cpu < 7 or avoid_cmpw:
            tryhop(2);POP()
        RET()

    module(name='rt_fdivloop.s',
           code=[ ('EXPORT', '__@fdivloop'),
                  ('EXPORT', '__@fdivrnd'),
                  ('IMPORT', '__@amshl1') if args.cpu < 7 else ('NOP',),
                  ('IMPORT', '__@cmshl1') if args.cpu < 6 else ('NOP',),
                  ('IMPORT', '__@lsub_t0t1') if args.cpu < 6 else ('NOP',),
                  ('IMPORT', '__@lcmpu_t0t1') if avoid_cmpw else ('NOP',),
                  ('CODE', '__@fdivloop', code_fdivloop),
                  ('CODE', '__@fdivrnd', code_fdivrnd) ] )

    def code_fdiv():
        '''_@_fdiv: Divide FAC by the float at address vAC.'''
        label('_@_fdiv')
        PUSH();STW(T3)
        _CALLJ('__@fsavevsp')
        if args.cpu >= 7:
            LDW(T3);LDFARG()
        else:
            _CALLJ('__@fldarg_t3')
        label('__@fdivfa')
        LD(AS);ANDI(1);ADDI(127);ST(AS)  # - sign
        LD(BE)
        if args.cpu >= 7:
            STW(BE)
        _BNE('.fdiv0')
        _CALLJ('__@fexception')          # - divisor is zero -> exception
        label('.fdiv0')
        LD(AE);_BEQ('.zero')             # - dividend is zero -> result is zero
        SUBW(BE);ADDI(160)
        _BLE('.zero')                    # - hopeless underflow
        ALLOC(-2);STLW(0)
        _CALLJ('__@fdivloop')
        _CALLJ('__@fdivrnd')
        LDLW(0);ALLOC(2)
        ST(AE);_BLE('.zero')             # - copy exponent
        LD(vACH);_BNE('.ovf')            #   and test for overflow
        ST(AM)                           # - copy quotient into AM
        _MOVW(CM, AM+1)
        _MOVW(CM+2, AM+3)
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
                  ('IMPORT', '__@fldarg_t3') if args.cpu < 7 else ('NOP',),
                  ('IMPORT', '__@fexception'),
                  ('IMPORT', '__@fdivloop'),
                  ('IMPORT', '__@fdivrnd'),
                  ('IMPORT', '_@_clrfac'),
                  ('IMPORT', '__@foverflow'),
                  ('CODE', '_@_fdiv', code_fdiv) ] )

    def code_fdivr():
        '''_@_fdiv: Divide the float at address vAC by FAC, result in FAC.'''
        label('_@_fdivr')
        PUSH();STW(T3)
        _CALLJ('__@fsavevsp')
        _CALLJ('__@fac2farg')
        if args.cpu >= 7:
            LDW(T3);LDFAC()
        else:
            _CALLJ('__@fldfac_t3')
        _CALLJ('__@fdivfa') # no return

    module(name='rt_fdivr.s',
           code=[ ('EXPORT', '_@_fdivr'),
                  ('IMPORT', '__@fsavevsp'),
                  ('IMPORT', '__@fac2farg'),
                  ('IMPORT', '__@fldfac_t3') if args.cpu < 7 else ('NOP',),
                  ('IMPORT', '__@fdivfa'),
                  ('CODE', '_@_fdivr', code_fdivr) ] )

    # ==== fmod

    def code_fmod():
        '''_@_fmod: Leaves in FAC the floating point remainder FAC % [vAC]
           that is FAC - n * [vAC] where n is the quotient FAC/[vAC]
           rounded toward zero to an integer. Returns the 16 low bits
           of n into vAC'''
        nohop()
        label('_@_fmod')
        PUSH();STW(T3)
        _CALLJ('__@fsavevsp')
        if args.cpu >= 7:
            LDW(T3);LDFARG()
        else:
            _CALLJ('__@fldarg_t3')
        LD(BE);STW(BE);_BEQ('.zero')
        LD(AE);_BEQ('.zero')
        SUBW(BE);_BLT('.zquo')
        ALLOC(-2);STLW(0)
        LD(BE);ST(AE)
        _CALLJ('__@fdivloop')
        LDLW(0);ALLOC(2)
        _BNE('.zero')
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
                  ('IMPORT', '__@fldarg_t3') if args.cpu < 7 else ('NOP',),
                  ('IMPORT', '__@fdivloop'),
                  ('IMPORT', '_@_clrfac'),
                  ('IMPORT', '__@fnorm') ] )

    # ==== comparisons

    def code_fcmp():
        '''_@_fcmp: Compare FAC with the float at address vAC and return -1,
           0, or +1.  Note that because of the absence of subnormal
           numbers, this function might declare that two very small
           numbers are different even though subtracting one from the
           other might underflow and return zero.'''
        label('_@_fcmp')
        if args.cpu >= 7:
            LDFARG()
        else:
            PUSH()
            STW(T3);_CALLJ('__@fldarg_t3')
        LD(BE);STW(BE);_BNE('.fcmp0')
        LD(AE);_BEQ('.zero')
        label('.plus')
        LD(AS);XORI(128);ANDI(128);PEEK();LSLW();SUBI(1)
        if args.cpu < 7:
            tryhop(2);POP()
        RET()
        label('.minus')
        LD(AS);ANDI(128);PEEK();LSLW();SUBI(1)
        if args.cpu < 7:
            tryhop(2);POP()
        RET()
        label('.fcmp0')     # comparing sign
        LD(AS);ANDI(1);_BNE('.plus')
        label('.fcmp1')     # comparing exponents
        LD(AE);SUBW(BE);_BGT('.plus');_BLT('.minus')
        label('.fcmp2')     # comparing mantissa
        if args.cpu >= 6:
            LDI(BM+1);CMPLU();_BLT('.minus');_BGT('.plus')
        elif avoid_cmpw:
            LDW(BM+1);STW(T0);LDW(BM+3);STW(T1)
            _CALLJ('__@lcmpu_t0t1');_BLT('.minus');_BGT('.plus')
        else:
            LDW(AM+3);_CMPWU(BM+3);_BLT('.minus');_BGT('.plus')
            LDW(AM+1);_CMPWU(BM+1);_BLT('.minus');_BGT('.plus')
        label('.zero')
        LDI(0)
        if args.cpu < 7:
            tryhop(2);POP()
        RET()

    module(name='rt_fcmp.s',
           code=[ ('EXPORT', '_@_fcmp'),
                  ('IMPORT', '_@_rndfac'),
                  ('IMPORT', '__@fldarg_t3') if args.cpu < 7 else ('NOP',),
                  ('IMPORT', '__@lcmpu_t0t1') if avoid_cmpw else ('NOP',),
                  ('CODE', '_@_fcmp', code_fcmp) ] )

    def code_fsign():
        '''_@_fsign: return the sign of FAC into AC (-1/0/+1)'''
        nohop()
        label('_@_fsign')
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
        LD(AS);XORI(0x81);ST(AS)
        RET()

    module(name='rt_fneg.s',
           code=[ ('EXPORT', '_@_fneg'),
                  ('CODE', '_@_fneg', code_fneg) ] )

    def code_frexp():
        '''_@_frexp: Return x in FAC and exp in vAC such
           that FAC = x * 2^exp with 0.5<=x<1.'''
        nohop()
        label('_@_frexp')
        LD(AE);_BEQ('.frexp2')
        SUBI(128);STW(T3)
        LDI(128);ST(AE)
        LDW(T3)
        label('.frexp2')
        RET()

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
        LD(AE);ADDW(T3);_BLE('.fscal1')
        ST(AE);LD(vACH);_BEQ('.fscal0')
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


    def code_fscald():
        '''_@_fscald: Given x in FAC and m >= 0 in vACL,
               returns y in FAC and n >= 0 in vAC such that x and y
               have the same exponent and x * 10^m = y * 2^n.
               Calling _@_fscalb afterwards gives x * 10^m.'''
        label('_@_fscald')
        PUSH()
        STW(T3);_BEQ('.ret');_MOVIW(0,T2)
        label('.loop')
        if args.cpu >= 7:
            MOVF(AM,BM)     # mantissa
            LDI(2);LSRXA()
            LD(BM);ADDW(AM);ST(AM);XORW(AM);_BEQ('.a0')
            INCVL(LAC)
            label('.a0')
            LDI(BM+1);ADDL()
        else:
            _CALLJ('__@am2bm40')
            LDWI('SYS_LSRW2_52');STW('sysFn');_CALLI('__@amshrx')
            _CALLJ('__@amaddbm40')
        LDW(AM+3);_BLT('.a1')
        if args.cpu >= 7:
            LDI(1);RORX()
        elif args.cpu == 6:
            _LDI(0xffff);RORX(cpu6exact=False)
        else:
            LDWI('SYS_LSRW1_48');STW('sysFn');_CALLI('__@amshrx')
            LD(AM+4);ORI(128);ST(AM+4)
        LDI(4);_BRA('.a2')
        label('.a1')
        LDI(3)
        label('.a2')
        ADDW(T2);STW(T2)
        label('.tst')
        if args.cpu >= 6:
            DBNE(T3,'.loop');
        else:
            LD(T3);SUBI(1);ST(T3);_BNE('.loop')
        _CALLJ('_@_rndfac')
        LDW(T2)
        label('.ret')
        tryhop(2);POP();RET()

    module(name='rt_fscald.s',
           code=[ ('EXPORT', '_@_fscald'),
                  ('IMPORT', '__@am2bm40') if args.cpu < 7 else ('NOP',),
                  ('IMPORT', '__@amshrx') if args.cpu < 7 else ('NOP',),
                  ('IMPORT', '__@amaddbm40') if args.cpu < 7 else ('NOP',),
                  ('IMPORT', '_@_rndfac'),
                  ('CODE', '_@_fscald', code_fscald) ] )


    def code_frndz():
        '''_@_frndz: Make FAC integer, rounding towards zero'''
        nohop()
        label('_@_frndz')
        PUSH()
        _CALLJ('__@fac2farg')
        LDWI(0xffff);ST(AM);STW(AM+1);STW(AM+3)
        LD(AE);SUBI(128);_BLE('.fmask1')
        _CALLI('__@amshra')
        label('.fmask1')
        LDI(255);XORW(AM);ANDW(BM);ST(AM)
        if args.cpu >= 6:
            NOTVL(LAC);LDI(BM+1);ANDL();LDW(AM+3)
        else:
            LDWI(0xffff);XORW(AM+1);ANDW(BM+1);STW(AM+1)
            LDWI(0xffff);XORW(AM+3);ANDW(BM+3);STW(AM+3)
        _BNE('.frndz1')
        LDI(0);ST(AS);ST(AE)
        label('.frndz1')
        tryhop(2);POP();RET()

    module(name='rt_frndz.s',
           code=[ ('IMPORT', '__@amshra'),
                  ('IMPORT', '__@fac2farg'),
                  ('EXPORT', '_@_frndz'),
                  ('CODE', '_@_frndz', code_frndz) ] )


# create all the modules
scope()

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
