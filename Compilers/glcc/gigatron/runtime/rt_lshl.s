
def scope():

    # ----------------------------------------
    # LSHL1 : LAC <-- LAC << 1
    # ----------------------------------------

    def code0():
        """Left shift LAC. No other side effect. """
        nohop()
        label('_@_lshl1')
        if args.cpu >= 6:
            LSLVL(LAC)
            warning('cpu6: use LSLVL instead of _@_lshl1')
        else:
            LDW(LAC+2);LSLW();STW(LAC+2)
            LDW(LAC);_BGE('.l1');INC(LAC+2)
            label('.l1')
            LSLW();STW(LAC)
        RET()

    module(name='rt_lshl1.s',
           code=[ ('EXPORT', '_@_lshl1'),
                  ('CODE', '_@_lshl1', code0) ])


    # ----------------------------------------
    # LSHL1_T0T1:   T0T1 <-- T0T1 << 1
    # ----------------------------------------

    def code1():
        """Left shift T0T1. No other side effect. """
        nohop()
        label('__@lshl1_t0t1')
        if args.cpu >= 6:
            LSLVL(T0)
            warning('cpu6: use LSLVL instead of _@_lshl1')
        else:
            LDW(T1);LSLW();STW(T1)
            LDW(T0);_BGE('.l1');INC(T1)
            label('.l1')
            LSLW();STW(T0)
        RET()

    module(name='rt_lshl1t0t1.s',
           code=[ ('EXPORT', '__@lshl1_t0t1'),
                  ('CODE', '__@lshl1_t0t1', code1) ] )

    # ----------------------------------------
    # LSHL : LAC <-- LAC << vAC
    # ----------------------------------------

    def code2():
        """Left shift LAC by vAC bits.
           Trashes T5 i.e. sysArgs[67] like the alternative opcode LSLXA.
        """
        label('_@_lshl')
        if args.cpu >= 7:
            warning('cpu7: use LSLXA instead of _@_lshl')
            MOVQB(0,LAX);LSLXA();RET()
        else:
            PUSH()
            ST(T5);ANDI(16);_BEQ('.l4')
            LDW(LAC);STW(LAC+2);LDI(0);STW(LAC)
            label('.l4')
            LD(T5);ANDI(8);_BEQ('.l5')
            LDW(LAC+1);STW(LAC+2);LD(LAC);ST(LAC+1);LDI(0);ST(LAC)
            label('.l5')
            LD(T5);ANDI(4);_BEQ('.l6')
            LDWI('SYS_LSLW4_46');STW('sysFn')
            LDW(LAC+2);SYS(46);LD(vACH);ST(LAC+3)
            LDW(LAC+1);SYS(46);LD(vACH);ST(LAC+2)
            LDW(LAC);SYS(46);STW(LAC)
            label('.l6')
            LD(T5);ANDI(3);_BEQ('.ret')
            label('.l7')
            ST(T5);
            if args.cpu >= 6:
                LSLVL(LAC)
            else:
                _CALLJ('_@_lshl1')
            LD(T5);SUBI(1);_BNE('.l7')
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
