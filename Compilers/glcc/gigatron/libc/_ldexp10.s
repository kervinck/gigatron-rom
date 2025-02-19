
def scope():

    def code_ldexp10p():
        label('_ldexp10p')
        # -- double _ldexp10p(double *x, int exp)
        # Function _ldexp10 is assumed
        # to keep the scratch registers intact.
        PUSH()
        LDW(R9);_BLT('.neg')
        SUBI(128);_BGE('.ovf')
        label('.pos0')
        LDW(R8);_MOVF([vAC],FAC)
        LDW(R9);_CALLI('_@_fscald')
        _BRA('.ret')
        label('.ovf')
        LDWI(1000);_BRA('.ret')
        label('.und')
        LDWI(-1000);_BRA('.ret')
        label('.neg')
        ADDI(128);_BLT('.und')
        _MOVF('_fone',FAC)
        LDI(0);SUBW(R9);_CALLI('_@_fscald');STW(R9)
        LDW(R8);_FDIVR();LDI(0);SUBW(R9)
        label('.ret')
        _FSCALB()
        tryhop(2);POP();RET()

    module(name='_ldexp10p',
           code=[ ('EXPORT', '_ldexp10p'),
                  ('IMPORT', '_@_fscald'),
                  ('IMPORT', '_@_fscalb'),
                  ('IMPORT', '_@_clrfac'),
                  ('IMPORT', '_fone'),
                  ('CODE', '_ldexp10p', code_ldexp10p) ] )


    def code_frexp10p():
        # -- int _frexp10p(double *x)
        label('_frexp10p')
        PUSH()
        LDW(R8);STW(R18);PEEK();_BEQ('.r0')
        SUBI(157);STW(R17);LSLW();ADDW(R17);_DIVIS(10);STW(R17)
        LDI(0);SUBW(R17);STW(R9);_CALLJ('_ldexp10p')
        label('.r3')
        LD(FAE);SUBI(157);_BGT('.r1');_BLT('.r2')
        LD(LAC+3);ORI(0x80);SUBI(0xcc);_BGE('.r1')
        label('.r2')
        LDI(1);_CALLI('_@_fscald');_FSCALB();
        LDW(R17);SUBI(1);STW(R17)
        _BRA('.r3')
        label('.r1')
        LDW(R18);_MOVF(FAC,[vAC]);LDW(R17);
        label('.r0')
        tryhop(2);POP();RET();

    module(name='_frexp10p',
           code=[ ('EXPORT', '_frexp10p'),
                  ('IMPORT', '_ldexp10p'),
                  ('IMPORT', '_@_fscald'),
                  ('CODE', '_frexp10p', code_frexp10p) ] )


scope()

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
