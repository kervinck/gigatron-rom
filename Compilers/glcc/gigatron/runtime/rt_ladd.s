def scope():

    # LAC + [vAC] --> LAC
    def code0():
        nohop()
        label('_@_ladd')
        # load arg into T0/T1
        STW(T3);DEEK();STW(T0)
        LDI(2);ADDW(T3);DEEK();STW(T1)
        label('__@ladd_t0t1')
        if args.cpu <= 5:
            # alternating pattern
            LD(LAC);ADDW(T0);ST(LAC);LD(vACH)
            BNE('.a1');LD(T0+1);BEQ('.a1');LDWI(0x100);label('.a1')
            ADDW(LAC+1);ST(LAC+1);LD(vACH)
            BNE('.a2');LD(LAC+2);BEQ('.a2');LDWI(0x100);label('.a2')
            ADDW(T0+2);ST(LAC+2);LD(vACH)
            BNE('.a3');LD(T0+3);BEQ('.a3');LDWI(0x100);label('.a3')
            ADDW(LAC+3);ST(LAC+3)
        else:
            # not used: cpu6 now emits ADDLP
            LD(LAC);ADDBA(T0);ST(LAC);LD(vACH)
            ADDBA(LAC+1);ADDBA(T0+1);ST(LAC+1);LD(vACH)
            ADDBA(LAC+2);ADDBA(T0+2);ST(LAC+2);LD(vACH)
            ADDBA(LAC+3);ADDBA(T0+3);ST(LAC+3)
        RET()

    module(name='rt_ladd.s',
           code= [ ('EXPORT', '_@_ladd'),
                   ('EXPORT', '__@ladd_t0t1'),
                   ('CODE', '_@_ladd', code0) ] )


    # LAC - [vAC] --> LAC
    def code0():
        nohop()
        label('_@_lsub')
        # load arg into T0/T1
        STW(T3);DEEK();STW(T0)
        LDI(2);ADDW(T3);DEEK();STW(T1)
        label('__@lsub_t0t1')
        if args.cpu <= 5:
            # alternating pattern
            LD(LAC);SUBW(T0);ST(LAC);LD(vACH)
            BNE('.a1');LD(T0+1);XORI(255);BEQ('.a1');LDWI(0x100);label('.a1')
            ADDW(LAC+1);ST(LAC+1);LD(vACH)
            BNE('.a2');LD(LAC+2);BEQ('.a2');LDWI(0x100);label('.a2')
            SUBI(1);SUBW(T0+2);ST(LAC+2);LD(vACH)
            BNE('.a3');LD(T0+3);XORI(255);BEQ('.a3');LDWI(0x100);label('.a3')
            ADDW(LAC+3);ST(LAC+3)
        else:
            # not used: cpu6 now emits SUBLP
            LD(LAC);SUBBA(T0);ST(LAC);LD(vACH);ST(vACH)
            ADDBA(LAC+1);SUBBA(T0+1);ST(LAC+1);LD(vACH);ST(vACH)
            ADDW(LAC+2);SUBW(T0+2);STW(LAC+2)
        RET()

    module(name='rt_lsub.s',
           code=[ ('EXPORT', '_@_lsub'),
                  ('EXPORT', '__@lsub_t0t1'),
                  ('CODE', '_@_lsub', code0) ])

    # -LAC --> LAC
    def code1():
        nohop()
        label('_@_lneg')
        _LDI(0xffff);XORW(LAC+2);STW(LAC+2)
        _LDI(0xffff);XORW(LAC);ADDI(1);STW(LAC)
        BNE('.lneg1')
        LDI(1);ADDW(LAC+2);STW(LAC+2)
        label('.lneg1')
        RET()

    module(name='rt_lneg.s',
           code= [ ('EXPORT', '_@_lneg'),
                   ('CODE', '_@_lneg', code1) ] )

    # LNEG_T0T1 : -TOT1 --> TOT1
    def code1():
        nohop()
        label('__@lneg_t0t1')
        _LDI(0xffff);XORW(T1);STW(T1)
        _LDI(0xffff);XORW(T0);ADDI(1);STW(T0)
        BNE('.lneg1')
        LDI(1);ADDW(T1);STW(T1)
        label('.lneg1')
        RET()

    module(name='rt_lnegt0t1.s',
           code=[ ('EXPORT', '__@lneg_t0t1'),
                  ('CODE', '__@lshl1_t0t1', code1) ])


scope()

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
