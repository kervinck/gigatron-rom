
def scope():

    # LSHL1 : LAC <-- LAC << 1
    def code0():
        nohop()
        label('_@_lshl1')
        if args.cpu >= 6:
            LDI(0)
            NROL(4,LAC)
            RET()
        else:
            LDW(LAC);BLT('.l1')
            LSLW();STW(LAC);LDW(LAC+2);LSLW();STW(LAC+2);RET()
            label('.l1')
            LSLW();STW(LAC);LDW(LAC+2);LSLW();ORI(1);STW(LAC+2)
            RET()

    module(name='rt_lshl1.s',
           code=[ ('EXPORT', '_@_lshl1'),
                  ('CODE', '_@_lshl1', code0) ])

    # LSHL1_T0T1:   T0T1 <-- T0T1 << 1
    def code1():
        nohop()
        label('__@lshl1_t0t1')
        if args.cpu >= 6:
            LDI(0)
            NROL(4,T0)
            RET()
        else:
            LDW(T0);BLT('.lsl1')
            LSLW();STW(T0);LDW(T0+2);LSLW();STW(T0+2);RET()
            label('.lsl1')
            LSLW();STW(T0);LDW(T0+2);LSLW();ORI(1);STW(T0+2)
            RET()

    module(name='rt_lshl1t0t1.s',
           code=[ ('EXPORT', '__@lshl1_t0t1'),
                  ('CODE', '__@lshl1_t0t1', code1) ] )

    # LSHL : LAC <-- LAC << AC  (clobbers B0,T2,T3)
    def code2():
        label('_@_lshl')
        PUSH()
        ST(B0);ANDI(16);_BEQ('.l4')
        LDW(LAC);STW(LAC+2);LDI(0);STW(LAC)
        label('.l4')
        LD(B0);ANDI(8);_BEQ('.l5')
        LDW(LAC+1);STW(LAC+2);LD(LAC);ST(LAC+1);LDI(0);ST(LAC)
        label('.l5')
        LD(B0);ANDI(4);_BEQ('.l6')
        LDWI('SYS_LSLW4_46');STW('sysFn')
        LDW(LAC+2);SYS(46);LD(vACH);ST(LAC+3)
        LDW(LAC+1);SYS(46);LD(vACH);ST(LAC+2)
        LDW(LAC);SYS(46);STW(LAC)
        label('.l6')
        LD(B0);ANDI(3);_BEQ('.ret')
        label('.l7')
        ST(B0);
        if args.cpu >= 6:
            NROL(4, LAC)
        else:
            _CALLJ('_@_lshl1')
        LD(B0);SUBI(1);_BNE('.l7')
        label('.ret')
        tryhop(2);POP();RET()

    module(name='rt_lshl.s',
           code=[ ('EXPORT', '_@_lshl'),
                  ('IMPORT', '_@_lshl1') if args.cpu < 6 else ('NOP',),
                  ('CODE', '_@_lshl', code2) ] )

scope()

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
