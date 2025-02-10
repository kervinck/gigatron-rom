

def scope():
   
    def code0():
        nohop()
        label('__@shrsysfn')
        LSLW()
        if args.cpu >= 6:
            ADDWI(v('.shrtable')-2)
        else:
            PUSH();STW(vLR);LDWI(v('.shrtable')-2);ADDW(vLR);POP()
        DEEK();STW('sysFn')
        RET()
        label(".shrtable")
        words("SYS_LSRW1_48")
        words("SYS_LSRW2_52")
        words("SYS_LSRW3_52")
        words("SYS_LSRW4_50")
        words("SYS_LSRW5_50")
        words("SYS_LSRW6_48")
        words('SYS_LSRW7_30')

    module(name='rt_shrtable.s',
           code=[('EXPORT', '__@shrsysfn'),
                 ('CODE', '__@shrsysfn', code0) ] )

    # SHRU: T3>>vAC -> vAC  (unsigned)
    # Trashes T5
    def code1():
        nohop()
        label('_@_shrs')
        ST(T5);LDW(T3);_BGE('.shr0')
        LDWI(0xffff);STW(T4);XORW(T3);STW(T3)
        BRA('.shr1')
        label('_@_shru')
        ST(T5)
        label('.shr0')
        LDI(0);STW(T4)
        label('.shr1')
        LD(T5);ANDI(8);_BEQ('.shr2')
        LD(T3+1);STW(T3)
        label('.shr2')
        if args.cpu < 5:
            LDWI('__@shrsysfn');STW('sysFn')
        LD(T5);ANDI(7);_BEQ('.shr3')
        PUSH()
        if args.cpu < 5:
            CALL('sysFn')
        else:
            CALLI('__@shrsysfn')
        POP();
        LDW(T3);SYS(52);BRA('.shr4')
        label('.shr3')
        LDW(T3)
        label('.shr4')
        XORW(T4);RET()

    module(name='rt_shr.s',
           code=[('EXPORT', '_@_shru'),
                 ('EXPORT', '_@_shrs'),
                 ('IMPORT', '__@shrsysfn'),
                 ('CODE', '_@_shr', code1) ] )


    # SHRU1/SHRS1 : AC <-- AC >> 1 (unsigned)
    # clobbers T[45]
    def code0():
        nohop()
        if args.cpu >= 7:
            label('_@_shrs1')
            _BGE('_@_shru1')
            MOVIW(0x8000,T4)
            BRA('.shr')
            label('_@_shru1')
            MOVQW(0,T4)
            label('.shr')
            MOVIW('SYS_LSRW1_48','sysFn')
        else:
            label('_@_shrs1')
            _BGE('_@_shru1')
            STW(T5);_LDI(0x8000);STW(T4)
            BRA('.shr')
            label('_@_shru1')
            STW(T5);LDI(0);STW(T4)
            label('.shr')
            LDWI('SYS_LSRW1_48');STW('sysFn')
            LDW(T5)
        SYS(48);ORW(T4)
        RET()

    module(name='rt_shr1.s',
           code=[('EXPORT', '_@_shrs1'),
                 ('EXPORT', '_@_shru1'),
                 ('CODE', '_@_shr1', code0) ] )
   
scope()

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
