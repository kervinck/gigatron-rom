

def scope():
   
    def code0():
        nohop()
        label('__@shrsysfn')
        PUSH();LSLW();STW(vLR)
        LDWI(v('.shrtable')-2)
        ADDW(vLR);DEEK();STW('sysFn')
        POP();RET()
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
        tryhop(3)
        label('_@_shru')
        ST(T5)
        label('__@shru_t5')
        PUSH()
        LD(T5);ANDI(8);_BEQ('.shru7')
        LD(T3+1);STW(T3)
        label('.shru7')
        LD(T5);ANDI(7);_BEQ('.shru1');
        _CALLI('__@shrsysfn')
        LDW(T3);SYS(52)
        tryhop(2);POP();RET()
        label('.shru1')
        LDW(T3)
        tryhop(2);POP();RET()

    module(name='rt_shru.s',
           code=[('EXPORT', '_@_shru'),
                 ('EXPORT', '__@shru_t5'),
                 ('IMPORT', '__@shrsysfn'),
                 ('CODE', '_@_shru', code1) ] )

    # SHRS: T3>>vAC -> vAC  (signed)
    # clobbers T5
    def code2():
       label('_@_shrs')
       PUSH();ST(T5)
       LDW(T3);_BGE('.shrs1')
       _LDI(0xffff);XORW(T3);STW(T3)
       _CALLJ('__@shru_t5')
       STW(T3);_LDI(0xffff);XORW(T3)
       tryhop(2);POP();RET()
       label('.shrs1')
       _CALLJ('__@shru_t5')
       label('.shrs2')
       tryhop(2);POP();RET()

    module(name='rt_shru.s',
           code=[('EXPORT', '_@_shrs'),
                 ('IMPORT', '__@shru_t5'),
                 ('CODE', '_@_shrs', code2) ] )

    # SHRU1/SHRS1 : AC <-- AC >> 1 (unsigned)
    # clobbers T[45]
    def code0():
        nohop()
        label('_@_shru1')
        if args.cpu >= 7:
            MOVIW('SYS_LSRW1_48','sysFn')
        else:
            STW(T5)
            _MOVIW('SYS_LSRW1_48','sysFn')
            LDW(T5)
        SYS(48)
        RET()
        label('_@_shrs1')
        _BGE('_@_shru1')
        if args.cpu >= 7:
            MOVIW('SYS_LSRW1_48','sysFn')
            MOVIW(0x8000, T4)
        else:
            STW(T5)
            _MOVIW('SYS_LSRW1_48','sysFn')
            _MOVIW(0x8000, T4)
            LDW(T5)
        SYS(48); ORW(T4)
        RET()

    module(name='rt_shr1.s',
           code=[('EXPORT', '_@_shrs1'),
                 ('EXPORT', '_@_shru1'),
                 ('CODE', '_@_shru1', code0) ] )
   
scope()

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
