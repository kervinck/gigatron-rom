
def scope():

    bigstep = 16
    bigcst = '.1e16'
    
    def code_ldexpcst():
        label('.1e16')
        bytes(182,14,27,201,191); # 1e+16
    
    def code_ldexp10n():
        label('.neg0')
        LDW(R11);ADDI(bigstep);_BGE('.neg2')
        STW(R11);_LDI(bigcst);_FDIV()
        _FSGN();_BNE('.neg0')
        tryhop(2);POP();RET()
        label('.neg2')
        LDI(0);SUBW(R11)
        STW(R11);_BEQ('.ret')
        _MOVF(FAC,F8)
        _MOVF('_fone', FAC)
        _CALLJ('.posf')
        LDI(F8);_FDIVR()
        label('.ret')
        tryhop(2);POP();RET()

    def code_ldexp10():
        label('_ldexp10')
        PUSH()
        _MOVF(F8,FAC)
        LDW(R11);_BGE('.pos0')
        _CALLJ('.neg0') # no return
        label('.pos0')
        LDW(R11);SUBI(bigstep);_BLE('.pos2')
        STW(R11);_LDI(bigcst);_FMUL()
        _BRA('.pos0')
        label('.posf')
        PUSH();LDW(R11)
        label('.pos1')
        SUBI(1);STW(R11);
        _CALLJ('_@_fmul10')
        label('.pos2')
        LDW(R11);_BGT('.pos1')
        _CALLJ('_@_rndfac')
        tryhop(2);POP();RET()

    module(name='_ldexp10',
           code=[ ('EXPORT', '_ldexp10'),
                  ('IMPORT', '_@_fmul10'),
                  ('IMPORT', '_@_rndfac'),
                  ('IMPORT', '_fone'),
                  ('DATA', '_ldexpcst', code_ldexpcst, 0, 1),
                  ('CODE', '_ldexp10n', code_ldexp10n), 
                  ('CODE', '_ldexp10', code_ldexp10) ] )

    def code_frexp10():
        label('_frexp10')
        # Calling _ldexp10 will not damage R16-R19
        PUSH()
        LDW(R11);STW(R18)
        LD(F8);_BEQ('.zero')
        SUBI(158);STW(R17);LSLW();ADDW(R17);_DIVIS(10);STW(R17)
        LDI(0);SUBW(R17);STW(R11);_CALLJ('_ldexp10')
        label('.loop1')
        _MOVF(FAC,F8)
        LD(F8);SUBI(160);_BGT('.fix1')
        ADDI(3);_BGT('.ret');_BLT('.fix0')
        LD(F8+1);ORI(0x80);SUBI(0xcc);_BGE('.ret')
        label('.fix0')
        LDI(1);_BRA('.fix2')
        label('.fix1')
        _LDI(-1);
        label('.fix2')
        STW(R11);LDW(R17);SUBW(R11);STW(R17)
        _CALLJ('_ldexp10');_BRA('.loop1')
        label('.zero')
        STW(R17);_CALLJ('_@_clrfac')
        label('.ret')
        LDW(R17);DOKE(R18)
        tryhop(2);POP();RET();

    module(name='_frexp10',
           code=[ ('EXPORT', '_frexp10'),
                  ('IMPORT', '_ldexp10'),
                  ('IMPORT', '_@_clrfac'),
                  ('CODE', '_frexp10', code_frexp10) ] )

scope()

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
