

def scope():

    def code_utwoa():
        '''Internal: _utwoa(int) converts a number in range 0..99 into two
           chars returned as the high and low part of vAC.'''
        nohop()
        label('_utwoa')
        LDWI(0x2f2f);STW(R9)
        LDW(R8)
        label('.l1')
        INC(R9+1);SUBI(10);_BGE('.l1')
        ADDI(10)
        label('.l2')
        INC(R9);SUBI(1);_BGE('.l2')
        LDW(R9)
        RET()

    module(name='utwoa.s',
           code=[('EXPORT', '_utwoa'),
                 ('CODE', '_utwoa', code_utwoa)  ] )

    def code_asc():
        ''' T0:  (input) end of string
            T1:  (inout,preserved) start of string '''
        nohop()
        label('.d1')
        if args.cpu >= 7:
            ADDSV(-1,T0);PEEKV(T0)
        else:
            LDW(T0);SUBI(1);STW(T0);PEEK()
        ADDI(0x30);POKE(T0)
        SUBI(0x3a);_BLT('.d2')
        ADDI(0x61);POKE(T0)
        label('_itoa.asc')
        label('.d2')
        LDW(T0);XORW(T1);_BNE('.d1')
        LDW(T1)
        RET()

    def code_prep():
        nohop()
        label('_itoa.prep')
        if args.cpu >= 6:
            STW(R10);POKEQ(0);
            SUBI(1);STW(T1);POKEQ(0)
        else:
            STW(R10);SUBI(1);STW(T1)
            LDI(0);POKE(R10);POKE(T1)
        if args.cpu < 5:
            LDWI('_itoa.ddab');STW(R23)
        RET()

    def code_ddab1():
        ''' T0:   (input) end of string
            T1:   (inout) start of string (decremented)
            T4:   (input,preserved) base
            vAC:  (input) carry (0 or 1) '''
        nohop()
        label('_itoa.ddab1')
        ST(T5+1)
        label('.t0')
        LDW(T0);XORW(T1);_BEQ('.t1')
        LD(T5+1);STW(T5)
        LDW(T0);SUBI(1);STW(T0);PEEK()
        LSLW();ADDW(T5);POKE(T0)
        SUBW(T4);_BLT('.t0')
        POKE(T0);INC(T5+1);_BRA('.t0')
        label('.t1')
        LD(T5+1);_BEQ('.t2')
        LDW(T0);SUBI(1);STW(T1)
        LDI(1);POKE(T1)
        label('.t2')
        RET()

    def code_ddab_sys():
        ''' vAC: (input) binary number
            R10: (input,preserved) end of string
            R11/R12: (used)'''
        nohop()
        label('_itoa.ddab')
        info = rominfo['has_SYS_DoubleDabble']
        addr = int(str(info['addr']),0)
        cycs = int(str(info['cycs']),0)
        STW(R11)
        MOVQB(16,R12)
        MOVW(R10,T0)
        _MOVIW(addr,'sysFn')
        label('.s1')
        LDW(R11);ADDV(R11);SYS(cycs)
        DBNE(R12,'.s1')
        RET()

    def code_ddab_vcpu():
        ''' vAC: (input) binary number
            R10: (input,preserved) end of string
            R11/R12: (used)'''
        nohop()
        label('_itoa.ddab')
        PUSH()
        STW(R11)
        if args.cpu >= 6:
            MOVQB(16,R12)
        else:
            LDI(256-16);ST(R12)
        if args.cpu < 5:
            LDWI('_itoa.ddab1');STW(R22)
        label('.s1')
        _MOVW(R10,T0)
        LD(R11+1);ANDI(0x80);PEEK()
        if args.cpu < 5:
            CALL(R22)
        else:
            CALLI('_itoa.ddab1')
        if args.cpu >= 7:
            LDW(R11);ADDV(R11)
        else:
            LDW(R11);LSLW();STW(R11)
        if args.cpu >= 6:
            DBNE(R12, '.s1')
        else:
            INC(R12);LD(R12);_BNE('.s1')
        _MOVW(R10,T0)
        tryhop(2);POP();RET()

    if args.cpu >= 7 and 'has_SYS_DoubleDabble' in rominfo:
        module(name='itoa_subs.s',
               code=[('EXPORT', '_itoa.ddab'),
                     ('EXPORT', '_itoa.asc'),
                     ('EXPORT', '_itoa.prep'),
                     ('CODE', '_itoa.ddab', code_ddab_sys),
                     ('CODE', '_itoa.prep', code_prep),
                     ('CODE', '_itoa.asc', code_asc) ] )
    else:
        module(name='itoa_subs.s',
               code=[('EXPORT', '_itoa.ddab'),
                     ('EXPORT', '_itoa.asc'),
                     ('EXPORT', '_itoa.prep'),
                     ('CODE', '_itoa.ddab1', code_ddab1),
                     ('CODE', '_itoa.ddab', code_ddab_vcpu),
                     ('CODE', '_itoa.prep', code_prep),
                     ('CODE', '_itoa.asc', code_asc) ] )


    def code_utoa():
        '''char *utoa(unsigned int value, char *bufend, int radix)'''
        label('_utoa')
        PUSH()
        _MOVW(R10,T4)
        if args.cpu < 5:
            LDWI('_itoa.prep');STW(R23)
        LDW(R9)
        if args.cpu < 5:
            CALL(R23) # prep
            LDW(R8);CALL(R23) # ddab
        else:
            CALLI('_itoa.prep')
            LDW(R8);CALLI('_itoa.ddab')
        _CALLJ('_itoa.asc')
        tryhop(2);POP();RET()

    module(name='utoa.s',
           code=[('EXPORT', '_utoa'),
                 ('IMPORT', '_itoa.ddab'),
                 ('IMPORT', '_itoa.asc'),
                 ('IMPORT', '_itoa.prep'),
                 ('CODE', '_utoa', code_utoa)] )


    def code_ultoa():
        '''char *_ultoa(unsigned long value, char *bufend, int radix)'''
        nohop()
        label('_ultoa')
        PUSH()
        _MOVW(R11,T4)
        if args.cpu < 5:
            LDWI('_itoa.prep');STW(R23)
        LDW(R10)
        if args.cpu < 5:
            CALL(R23) # prep
            LDW(R9);CALL(R23)
            LDW(R8);CALL(R23)
        else:
            CALLI('_itoa.prep')
            LDW(R9);CALLI('_itoa.ddab')
            LDW(R8);CALLI('_itoa.ddab')
        _CALLJ('_itoa.asc')
        tryhop(2);POP();RET()

    module(name='ultoa.s',
           code=[('EXPORT', '_ultoa'),
                 ('IMPORT', '_itoa.ddab'),
                 ('IMPORT', '_itoa.asc'),
                 ('IMPORT', '_itoa.prep'),
                 ('CODE', '_ultoa', code_ultoa)] )


    def code_itoa():
        '''char *_itoa(int value, char *bufend, int radix)'''
        nohop()
        label('_itoa')
        if args.cpu >= 6:
            LDW(R8);JGE('_utoa')
            PUSH()
            NEGV(R8);CALLI('_utoa')
            SUBI(1);POKEQ(45)
            tryhop(2);POP();RET()
        else:
            PUSH();
            LDW(R8);_BLT('.neg')
            _CALLJ('_utoa')
            tryhop(2);POP();RET()
            label('.neg')
            LDI(0);SUBW(R8);STW(R8)
            _CALLJ('_utoa')
            SUBI(1);STW(R9)
            LDI(45);POKE(R9);LDW(R9)
            tryhop(2);POP();RET()

    module(name="itoa.s",
           code=[('EXPORT', '_itoa'),
                 ('IMPORT', '_utoa'),
                 ('CODE', '_itoa', code_itoa)] )


    def code_ltoa():
        '''char *_ltoa(long value, char buffer[16], int radix)'''
        nohop()
        label('_ltoa')
        if args.cpu >= 6:
            LDW(L8+2);JGE('_ultoa')
            PUSH()
            NEGVL(L8);CALLI('_ultoa')
            SUBI(1);POKEQ(45)
            tryhop(2);POP();RET()
        else:
            PUSH();
            LDW(L8+2);_BLT('.neg')
            _CALLJ('_ultoa')
            tryhop(2);POP();RET()
            label('.neg')
            LDI(0);SUBW(L8);STW(L8);_BEQ('.n1');LDWI(0xffff)
            label('.n1');SUBW(L8+2);STW(L8+2)
            _CALLJ('_ultoa')
            SUBI(1);STW(R9)
            LDI(45);POKE(R9);LDW(R9)
        tryhop(2);POP();RET()

    module(name="ltoa.s",
           code=[('EXPORT', '_ltoa'),
                 ('IMPORT', '_ultoa'),
                 ('CODE', '_ltoa', code_ltoa)] )


    def code_ftoa():
        ''' char *_ftoa(double *x, char *bufend)
            Return up to 10 digits representing the integer part of x.
            The final byte at address bufend will be either 0 or 0x80
            if the fractional part suggests rounding up. '''
        label('_ftoa')
        PUSH()
        LDW(R8);_MOVF([vAC],FAC)
        LDI(0);ST(FAS)
        _FTOU()
        # prep
        if args.cpu < 5:
            LDWI('_itoa.prep');STW(R23)
        _MOVIW(10,T4)
        LDW(R9)
        if args.cpu < 5:
            CALL(R23) # prep
            LDW(LAC+2);CALL(R23) # ddab
            LDW(LAC);CALL(R23) # ddab
        else:
            CALLI('_itoa.prep')
            LDW(LAC+2);CALLI('_itoa.ddab')
            LDW(LAC);CALLI('_itoa.ddab')
        LD(LAX);ANDI(0x80);POKE(R10)
        _CALLJ('_itoa.asc')
        tryhop(2);POP();RET()

    module(name='ftoa.s',
           code=[('EXPORT', '_ftoa'),
                 ('IMPORT', '_itoa.prep'),
                 ('IMPORT', '_itoa.ddab'),
                 ('IMPORT', '_itoa.asc'),
                 ('CODE', '_ftoa', code_ftoa) ] )

    def code_rnda():
        '''int _rnda(char *s, char *r);
           Truncate the string of digits <s> at position <r>.
           A zero termination will be written at position <r> and
           the string of digits will be rounded up if the prevous
           byte at position <r> was greater than or equal to '5'.
           Rounding may cause an extra digit to be preprended
           to the string, in which case the function returns 1.
           It returns zero otherwise. '''
        nohop()

        label('_rnda')
        _MOVIW(0x30,R11)
        _PEEKV(R9);SUBI(0x35);STW(R10);_BLT('.r0');INC(R11)
        label('.r0')
        LDI(0);POKE(R9)
        label('.r1')
        if args.cpu >= 7:
            LDW(R9);ADDSV(-1,R9)
        else:
            LDW(R9);SUBI(1);STW(R9);ADDI(1)
        XORW(R8);_BNE('.r2')
        LD(R11);POKE(R9);LDI(1);RET()
        label('.r2')
        LDW(R10);_BLT('.r3')
        LDW(R9);PEEK();ADDI(1);POKE(R9);SUBI(0x3a);_BLT('.r3')
        _MOVIW(0x31,R11)
        LDI(0x30);POKE(R9);_BRA('.r1')
        label('.r3')
        LDI(0);RET()

    module(name='rnda.s',
           code=[('EXPORT', '_rnda'),
                 ('CODE', '_rnda', code_rnda) ] )


scope()
# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
