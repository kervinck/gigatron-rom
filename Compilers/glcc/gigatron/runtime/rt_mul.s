def scope():


    # ----------------------------------------
    # MUL: T3 * vAC -> vAC
    # ----------------------------------------

    
    if 'has_SYS_Multiply_s16' in rominfo:
        info = rominfo['has_SYS_Multiply_s16']
        addr = int(str(info['addr']),0)
        cycs = int(str(info['cycs']),0)
        def code1():
            """ This is the dev7 SYS_Multiply_s16 version of MUL
                which requires less setup than the old version.
                Trashes T[01] sysArgs[0-7], sysFn. """
            nohop()
            label('_@_mul')             # T3*vAC -> vAC  (traditional entry point)
            STW('sysArgs2')
            LDW(T3);STW('sysArgs0')
            BRA('.1')
            label('_@_at67_mul')        # sysArgs0*vAC -> vAC  (faster entry point)
            STW('sysArgs2')
            label('.1')
            LDWI(addr);STW('sysFn')
            LDI(0);STW('sysArgs4')
            SYS(cycs)
            RET()

        module(name='rt_at67_mul.s',
               code= [ ('EXPORT', '_@_mul'),
                       ('EXPORT', '_@_at67_mul'),
                       ('CODE', '_@_mul', code1) ] )
    
    elif 'has_at67_SYS_Multiply_s16' in rominfo:
        # Multiply using SYS call
        info = rominfo['has_at67_SYS_Multiply_s16']
        addr = int(str(info['addr']),0)
        cycs = int(str(info['cycs']),0)
        def code1():
            """ This is the v6 SYS_Multiply_s16 version of MUL
                which requires more setup than the old version.
                Trashes sysArgs[0-7], sysFn. """
            nohop()
            label('_@_mul')             # T3*vAC -> vAC  (traditional entry point)
            STW('sysArgs2')
            LDW(T3);STW('sysArgs0')
            BRA('.1')
            label('_@_at67_mul')        # sysArgs0*vAC -> vAC  (faster entry point)
            STW('sysArgs2')
            label('.1')
            LDWI(addr);STW('sysFn')
            LDI(0);STW('sysArgs4');LDI(1);STW('sysArgs6')
            SYS(cycs)
            RET()

        module(name='rt_mul.s',
               code= [ ('EXPORT', '_@_mul'),
                       ('EXPORT', '_@_at67_mul'),
                       ('CODE', '_@_mul', code1) ] )

    else:
        # Muliply using vCPU
        def code0():
            """ This is the vCPU multiplication.
                Trashes sysArgs[0-7]. """
            nohop()
            label('_@_mul')             # T3*vAC --> vAC
            _BGE('.mul1');_BEQ('.ret')
            label('.mul2')
            STW(T0);LDW(T3);STW(T1);_BNE('.go');RET()
            label('.mul1')
            STW(T1);LDW(T3);STW(T0);_BEQ('.ret')
            label('.go')
            LDI(0);STW(T4);_LDI(0xffff);STW(T5)
            label('.loop')
            LDI(0);SUBW(T5);ANDW(T1);_BEQ('.shift')
            LDW(T4);ADDW(T0);STW(T4)
            label('.shift')
            LDW(T0);LSLW();STW(T0)
            LDW(T5);LSLW();STW(T5);ANDW(T1);_BNE('.loop')
            LDW(T4)
            label('.ret')
            RET()
      
        module(name='rt_mul.s',
               code= [ ('EXPORT', '_@_mul'),
                       ('CODE', '_@_mul', code0) ] )


scope()

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
