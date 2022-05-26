
def scope():

    # LSHRU : LAC <-- LAC >> AC (clobbers B0)
    def code0():
        nohop()
        label('_@_lshru')
        ST(B0)
        label('__@lshru_b0')
        PUSH()
        if args.cpu >= 6:
            LD(B0);XORI(1);_BNE('.not1')
            NROR(4, LAC);_BRA('.ret')
            label('.not1')
        LD(B0);ANDI(16);_BEQ('.l4')
        LDW(LAC+2);STW(LAC);LDI(0);STW(LAC+2)
        label('.l4')
        LD(B0);ANDI(8);_BEQ('.l5')
        LDW(LAC+1);STW(LAC);LD(LAC+3);STW(LAC+2)
        label('.l5')
        LD(B0);ANDI(7);_BEQ('.ret')
        _CALLI('__@shrsysfn')
        LDW(LAC);SYS(52);ST(LAC)
        LDW(LAC+1);SYS(52);ST(LAC+1)
        LDW(LAC+2);SYS(52);STW(LAC+2)
        label('.ret')
        tryhop(2);POP();RET()

    module(name='rt_lshru.s',
           code= [ ('EXPORT', '_@_lshru'),
                   ('EXPORT', '__@lshru_b0'),
                   ('IMPORT', '__@shrsysfn'),
                   ('CODE', '_@_lshru', code0) ])

    # LSHRS : LAC <-- LAC >> AC (signed) (clobbers T0, B0)
    def code1():
        label('_@_lshrs')
        PUSH();ST(B0)
        LDW(LAC+2);_BLT('.s1')
        _CALLJ('__@lshru_b0');_BRA('.sret')
        label('.s1')
        if args.cpu >= 6:
            NOTL(LAC)
        else:
            _LDI(0xffff);STW(T0);XORW(LAC);STW(LAC)
            LDW(T0);XORW(LAC+2);STW(LAC+2)
        _CALLJ('__@lshru_b0')
        if args.cpu >= 6:
            NOTL(LAC)
        else:
            LDW(T0);XORW(LAC);STW(LAC)
            LDW(T0);XORW(LAC+2);STW(LAC+2)
        label('.sret')
        tryhop(2);POP();RET()

    module(name='rt_lshrs.s',
           code= [ ('EXPORT', '_@_lshrs'),
                   ('IMPORT', '__@lshru_b0'),
                   ('CODE', '_@_lshrs', code1) ])

scope()


# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
