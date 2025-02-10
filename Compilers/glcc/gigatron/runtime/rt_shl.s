def scope():

    # T3<<vAC -> vAC
    def code0():
        nohop()
        label('_@_shl')
        if args.cpu >= 6:
            ANDI(0xf);SUBI(8);_BLT('.nxt')
            if args.cpu >= 7:
                MOVW(T3-1,T3);MOVQB(0,T3)
            else:
                STW(T2);LDW(T3-1);STW(T3);LD(T2)
            label('.nxt')
            ANDI(7);_BEQ('.ret3');ST(T2);LDW(T3)
            label('.loop')
            LSLW();DBNE(T2,'.loop')
            RET()
        else:
            STW(T2);_BEQ('.ret3')
            ANDI(0x8);_BEQ('.try4')
            LDW(T3-1);STW(T3) # high byte of T2 is zero!
            label('.try4')
            LD(T2);ANDI(0x4);_BEQ('.try2')
            LDWI('SYS_LSLW4_46');STW('sysFn');LDW(T3);SYS(46);STW(T3)
            label('.try2')
            LD(T2);ANDI(0x2);_BEQ('.try1')
            LDW(T3);LSLW();LSLW();STW(T3)
            label('.try1')
            LD(T2);ANDI(0x1);_BEQ('.ret3')
            LDW(T3);LSLW();RET()
        label('.ret3')
        LDW(T3)
        RET()

    module(name='rt_shl.s',
           code=[ ('EXPORT', '_@_shl'),
                  ('CODE', '_@_shl', code0) ] )

scope()

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
