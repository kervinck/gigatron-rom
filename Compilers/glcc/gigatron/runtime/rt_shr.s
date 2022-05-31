

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

    # SHRU: T3<<vAC -> vAC  (unsigned)
    def code1():
        tryhop(3)
        label('_@_shru')
        ST(T2)
        label('__@shru_t2')
        PUSH()
        LD(T2);ANDI(8);_BEQ('.shru7')
        LD(T3+1);STW(T3)
        label('.shru7')
        LD(T2);ANDI(7);_BEQ('.shru1');
        _CALLI('__@shrsysfn')
        LDW(T3);SYS(52)
        tryhop(2);POP();RET()
        label('.shru1')
        LDW(T3)
        tryhop(2);POP();RET()

    module(name='rt_shru.s',
           code=[('EXPORT', '_@_shru'),
                 ('EXPORT', '__@shru_t2'),
                 ('IMPORT', '__@shrsysfn'),
                 ('CODE', '_@_shru', code1) ] )

    # SHRS: T3<<vAC -> vAC  (signed)
    # clobbers T0
    def code2():
       label('_@_shrs')
       PUSH();ST(T2)
       LDW(T3);_BGE('.shrs1')
       if args.cpu >= 6:
           NOTW(T3)
       else:
           _LDI(0xffff);XORW(T3);STW(T3)
       _CALLJ('__@shru_t2')
       if args.cpu >= 6:
           NOTW(vAC)
       else:
           STW(T3);_LDI(0xffff);XORW(T3)
       _BRA('.shrs2')
       label('.shrs1')
       _CALLJ('__@shru_t2')
       label('.shrs2')
       tryhop(2);POP();RET()

    module(name='rt_shru.s',
           code=[('EXPORT', '_@_shrs'),
                 ('IMPORT', '__@shru_t2'),
                 ('CODE', '_@_shrs', code2) ] )

    # SHRU1/SHRS1 : AC <-- AC >> 1 (unsigned)
    def code0():
        nohop()
        label('_@_shru1')
        if args.cpu >= 6:
            LSRV(vAC)
        else:
            STW(T3); LDWI('SYS_LSRW1_48'); STW('sysFn'); LDW(T3)
            SYS(48)
        RET()
        label('_@_shrs1')
        BGE('_@_shru1')
        if args.cpu >= 6:
            LSRV(vAC);ORWI(0x8000)
        else:
            STW(T3); LDWI('SYS_LSRW1_48'); STW('sysFn'); LDWI(0x8000); STW(T2); LDW(T3)
            SYS(48); ORW(T2)
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
