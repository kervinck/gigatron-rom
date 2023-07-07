def scope():

    # LAC + [vAC] --> LAC
    def code0():
        nohop()
        if args.cpu >= 6:
            label('__@ladd_t0t1')
            LDI(T0)
            label('_@_ladd')
            ADDL();RET()
            warning("Cpu6: should ADDL instead of calling _@_ladd")
        else:
            label('_@_ladd')
            # load arg into T0/T1
            STW(T3);DEEK();STW(T0)
            LDI(2);ADDW(T3);DEEK();STW(T1)
            label('__@ladd_t0t1')
            LDW(LAC);ADDW(T0);STW(LAC);_BLT('.a1')
            SUBW(T0);ORW(T0);BRA('.a2')
            label('.a1')
            SUBW(T0);ANDW(T0)
            label('.a2')
            LD(vACH);ANDI(128);PEEK()
            ADDW(LAC+2);ADDW(T1);STW(LAC+2)
            RET()

    module(name='rt_ladd.s',
           code= [ ('EXPORT', '_@_ladd'),
                   ('EXPORT', '__@ladd_t0t1'),
                   ('CODE', '_@_ladd', code0) ] )


    # LAC - [vAC] --> LAC
    def code0():
        nohop()
        if args.cpu >= 6:
            label('__@lsub_t0t1')
            LDI(T0)
            label('_@_lsub')
            SUBL();RET()
            warning("Cpu6: should SUBL instead of calling _@_lsub")
        else:
            label('_@_lsub')
            # load arg into T0/T1
            STW(T3);DEEK();STW(T0)
            LDI(2);ADDW(T3);DEEK();STW(T1)
            label('__@lsub_t0t1')
            LDW(LAC);_BLT('.a1')
            SUBW(T0);STW(LAC);ORW(T0);BRA('.a2')
            label('.a1')
            SUBW(T0);STW(LAC);ANDW(T0)
            label('.a2')
            LD(vACH);ANDI(128);PEEK();XORI(1);SUBI(1)
            ADDW(LAC+2);SUBW(T1);STW(LAC+2)
            RET()

    module(name='rt_lsub.s',
           code=[ ('EXPORT', '_@_lsub'),
                  ('EXPORT', '__@lsub_t0t1'),
                  ('CODE', '_@_lsub', code0) ])

    # -LAC --> LAC
    def code1():
        nohop()
        label('_@_lneg')
        if args.cpu >= 6:
            NEGVL(LAC);RET()
            warning("Cpu6: should NEGVL instead of calling _@_lneg")
        else:
            LDI(0);SUBW(LAC);STW(LAC);BNE('.lneg1')
            LDI(0);SUBW(LAC+2);STW(LAC+2);RET()
            label('.lneg1')
            _LDI(-1);XORW(LAC+2);STW(LAC+2);RET()
            
    module(name='rt_lneg.s',
           code= [ ('EXPORT', '_@_lneg'),
                   ('CODE', '_@_lneg', code1) ] )

    # LNEG_T0T1 : -TOT1 --> TOT1
    def code1():
        nohop()
        label('__@lneg_t0t1')
        if args.cpu >= 6:
            NEGVL(T0);RET()
            warning("Cpu6: should NEGVL instead of calling __@lneg_t0t1")
        else:
            LDI(0);SUBW(T0);STW(T0);BNE('.lneg1')
            LDI(0);SUBW(T0+2);STW(T0+2);RET()
            label('.lneg1')
            _LDI(-1);XORW(T0+2);STW(T0+2);RET()

    module(name='rt_lnegt0t1.s',
           code=[ ('EXPORT', '__@lneg_t0t1'),
                  ('CODE', '__@lneg_t0t1', code1) ])


scope()

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
