def scope():

    # T3<<vAC -> vAC
    def code0():
        label('_@_shl')
        PUSH();STW(T2);_BEQ('.ret')
        ANDI(0x8);_BEQ('.try4')
        LDW(T3-1);STW(T3) # high byte of T2 is zero!
        label('.try4')
        LD(T2);ANDI(0x4);_BEQ('.try2')
        LDWI('SYS_LSLW4_46');STW('sysFn');LDW(T3);SYS(46);STW(T3)
        label('.try2')
        LD(T2);ANDI(0x2);_BEQ('.try1')
        LDW(T3);LSLW();LSLW();STW(T3)
        label('.try1')
        LD(T2);ANDI(0x1);_BEQ('.ret')
        LDW(T3);LSLW();_BRA('.reta')
        label('.ret')
        LDW(T3)
        label('.reta')
        tryhop(2);POP();RET()

    module(name='rt_shl.s',
           code=[ ('EXPORT', '_@_shl'),
                  ('CODE', '_@_shl', code0) ] )

scope()

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
