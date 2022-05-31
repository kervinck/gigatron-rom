

# char *utoa(unsigned int value, char *buffer, int radix)
# char *itoa(int value, char *buffer, int radix)
# char *ultoa(unsigned long value, char *buffer, int radix)
# char *ltoa(long value, char *buffer, int radix)

def scope():

    def code_utoa():
        label('utoa')
        PUSH()
        LDI(7);ADDW(R9);STW(R9)
        LDI(0);POKE(R9);_BRA('.loop')
        label('_utoa')
        PUSH()
        label('.loop')
        LDW(R9);SUBI(1);STW(R9)
        LDW(R8);_MODU(R10)
        SUBI(10);_BGE('.letter')
        ADDI(48+10);_BRA('.poke')
        label('.letter')
        ADDI(97);
        label('.poke')
        POKE(R9)
        LDW(T1);STW(R8);_BNE('.loop')
        tryhop(4);LDW(R9);POP();RET()

    module(name="utoa.s",
           code=[('EXPORT', 'utoa'),
                 ('EXPORT', '_utoa'),
                 ('CODE', 'utoa', code_utoa)] )

    def code_itoa():
        label('itoa')
        PUSH();
        LDI(0);SUBW(R8);_BGT('.neg')
        _CALLJ('utoa')
        tryhop(2);POP();RET()
        label('.neg')
        STW(R8)
        _CALLJ('utoa')
        SUBI(1);STW(R9);LDI(45);POKE(R9);
        LDW(R9)
        tryhop(2);POP();RET()
    
    module(name="itoa.s",
           code=[('EXPORT', 'itoa'),
                 ('IMPORT', 'utoa'),
                 ('CODE', 'itoa', code_itoa)] )

    def code_ultoa():
        label('ultoa')
        PUSH()
        LDI(15);ADDW(R10);STW(R10)
        LDI(0);POKE(R10);STW(R12)
        _MOVL(L8,LAC);
        label('.loop')
        LDI(R11);_LMODU()
        LDW(LAC);STW(R13)
        LDW(R10);SUBI(1);STW(R10)
        LDW(R13);SUBI(10);_BGE('.letter')
        ADDI(48+10);_BRA('.poke')
        label('.letter')
        ADDI(97);
        label('.poke')
        POKE(R10)
        LDW(T0);STW(LAC);LDW(T1);STW(LAC+2)
        ORW(T0);_BNE('.loop')
        LDW(R10)
        tryhop(2);POP();RET()
        
    module(name="ultoa.s",
           code=[('EXPORT', 'ultoa'),
                 ('CODE', 'ultoa', code_ultoa)] )

    def code_ltoa():
        label('ltoa')
        PUSH();
        LDW(L8+2);_BLT('.neg')
        _CALLJ('ultoa')
        tryhop(2);POP();RET()
        label('.neg')
        _MOVL(L8,LAC);_LNEG();_MOVL(LAC,L8)
        _CALLJ('ultoa')
        SUBI(1);STW(R9);LDI(45);POKE(R9);
        LDW(R9)
        tryhop(2);POP();RET()
    
    module(name="ltoa.s",
           code=[('EXPORT', 'ltoa'),
                 ('IMPORT', 'ultoa'),
                 ('CODE', 'ltoa', code_ltoa)] )

    def code_utwoa():
        '''Internal: _utwoa(int) converts a number in range 0..99 into two
           chars returned as the high and low part of vAC.'''
        label('_utwoa')
        PUSH()
        LDW(R8);_MODIU(10)
        ADDI(48);ST(R8)
        LD(T1);ADDI(48);ST(R8+1)
        LDW(R8)
        tryhop(2);POP();RET()

    module(name='utwoa.s',
           code=[('EXPORT', '_utwoa'),
                 ('CODE', '_utwoa', code_utwoa)  ] )

    def code_uftoa():
        '''Internal: _uftoa(double x, char *buf) does the same as
           _ultoa((unsigned long)x, buf, 10) but using _@_fmod instead
           of a long division. This code relies on the internal
           details of utoa. Beware.'''
        label('_uftoa')
        PUSH()
        _MOVF(F8,FAC)
        LDI(10);STW(R10)
        LDW(R11);ADDI(11);STW(R22);ADDI(4);STW(R9)
        LDI(0);POKE(R9)
        _LDI('.1e8');_CALLI('_@_fmod');STW(R20)
        _LDI('.1e4');_CALLI('_@_fmod');STW(R19)
        _FTOU()
        LDW(LAC);STW(R8);_CALLJ('_utoa')
        LDW(R19);_CALLI('.sub')
        LDW(R22);SUBI(4);STW(R22)
        LDW(R20)
        tryhop(2)
        POP()
        label('.sub',hop=0)
        PUSH()
        STW(R8);_BEQ('.ret')
        label('.loop')
        LDW(R9);SUBW(R22);_BLE('.sub1')
        LDW(R9);SUBI(1);STW(R9)
        LDI(48);POKE(R9)
        _BRA('.loop')
        label('.sub1')
        _CALLJ('_utoa')
        label('.ret')
        tryhop(4);LDW(R9);POP();RET()

    def code_uftoa_cst():
        label('.1e4')
        bytes(142,28,64,0,0) # 1e4
        label('.1e8')
        bytes(155,62,188,32,0) # 1e+08

    module(name='uftoa.s',
           code=[('EXPORT', '_uftoa'),
                 ('CODE', '_uftoa', code_uftoa),
                 ('DATA', '_uftoa', code_uftoa_cst, 0, 1),
                 ('IMPORT', '_@_fmod'),
                 ('IMPORT', '_utoa') ] )

scope()
# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
