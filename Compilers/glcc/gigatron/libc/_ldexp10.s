
def scope():

    def code_ldexp10():
        label('_ldexp10')
        PUSH()
        _MOVF(F8,FAC)
        LDW(R11);_BLT('.neg0')
        SUBI(128);_BLT('.pos0')
        LDWI(1000);_BRA('.pos1')
        label('.pos0')
        LDW(R11);_CALLI('_@_fscald')
        label('.pos1')
        _FSCALB()
        tryhop(2);POP();RET()
        label('.neg0')
        ADDI(128);_BGT('.neg1')
        _CALLJ('_@_clrfac')
        tryhop(2);POP();RET()
        label('.neg1')
        _MOVF('_fone',FAC);
        LDI(0);SUBW(R11);_CALLI('_@_fscald')
        STW(R12)
        LDI(F8);_FDIVR()
        LDI(0);SUBW(R12);_FSCALB()
        tryhop(2);POP();RET()

    module(name='_ldexp10',
           code=[ ('EXPORT', '_ldexp10'),
                  ('IMPORT', '_@_fscald'),
                  ('IMPORT', '_@_fscalb'),
                  ('IMPORT', '_@_clrfac'),
                  ('IMPORT', '_fone'),
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
