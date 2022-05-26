def scope():

    if 'has_at67_SYS_Multiply_s16' in rominfo:
        # Muliply using SYS call
        info = rominfo['has_at67_SYS_Multiply_s16']
        addr = int(str(info['addr']),0)
        cycs = int(str(info['cycs']),0)
        def code1():
            nohop()
            label('_@_mul')             # T3*vAC -> vAC  (traditional entry point)
            STW('sysArgs2')
            LDW(T3);STW('sysArgs0')
            BRA('.1')
            label('_@_at67_mul')        # sysArgs0*vAC -> vAC  (faster entry point)
            STW('sysArgs2')
            label('.1')
            LDWI(addr);STW('sysFn')
            if args.cpu >= 6:
                MOVQW(0, 'sysArgs4');MOVQW(1, 'sysArgs6')
            else:
                LDI(0);STW('sysArgs4');LDI(1);STW('sysArgs6')
            SYS(cycs)
            LDW('sysArgs4')
            RET()

        module(name='rt_at67_mul.s',
               code= [ ('EXPORT', '_@_mul'),
                       ('EXPORT', '_@_at67_mul'),
                       ('CODE', '_@_mul', code1) ] )

    else:
        # Muliply using vCPU
        def code0():
            nohop()
            label('_@_mul')             # T3*vAC --> vAC
            STW(T2)
            # T3: a, T2: b, T1: mask, T0: res
            LDI(0);STW(T0)
            LDW(T2);_BEQ('.ret');_BGE('.go')
            STW(T1);LDW(T3);STW(T2);LDW(T1);STW(T3)
            label('.go')
            _LDI(0xffff);STW(T1)
            label('.loop')
            LDI(0);SUBW(T1);ANDW(T2);_BEQ('.shift')
            LDW(T3);ADDW(T0);STW(T0)
            label('.shift')
            LDW(T3);LSLW();STW(T3)
            LDW(T1);LSLW();STW(T1);ANDW(T2);_BNE('.loop')
            label('.ret')
            LDW(T0);RET()
      
        module(name='rt_mul.s',
               code= [ ('EXPORT', '_@_mul'),
                       ('CODE', '_@_mul', code0) ] )


scope()

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
