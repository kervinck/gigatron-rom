
def scope():

    # Long division helper using vCPU
    #  LAC:  in: dividend  out: remainder
    #  T0T1: in: divisor
    #  T2T3: out: quotient
    # Clobbers sysArgs4

    def CallWorker():
        _CALLJ('__@ldivworker')

    def code_ldivworker():
        nohop()                    # 92 bytes
        label('__@ldivworker')
        if args.cpu < 6:
            PUSH()
        _MOVW(LAC,T2);_MOVW(LAC+2,T2+2)
        LDI(0);STW(LAC);STW(LAC+2)
        label('.ldiv0')
        if args.cpu >= 6:
            LSLVL(LAC)
            LDW(T2+2);_BGE('.ldw1');INC(LAC)
            label('.ldw1')
            LSLVL(T2)
        else:
            LDW(LAC+2);LSLW();STW(LAC+2)
            LDW(LAC);_BGE('.ldw1');INC(LAC+2)
            label('.ldw1');LSLW();STW(LAC)
            LDW(T2+2);_BGE('.ldw2');INC(LAC)
            label('.ldw2');LSLW();STW(T2+2)
            LDW(T2);_BGE('.ldw3');INC(T2+2)
            label('.ldw3');LSLW();STW(T2)
        if args.cpu >= 6:
            LDI(T0);CMPLU()
        else:
            _CALLJ('__@lcmpu_t0t1')
        _BLT('.ldiv1')
        if args.cpu >= 6:
            LDI(T0);SUBL()
        else:
            _CALLJ('__@lsub_t0t1')
        INC(T2)
        label('.ldiv1')
        if args.cpu >= 6:
            DBNE(T4,'.ldiv0')
        else:
            INC(T4);LD(T4);_BNE('.ldiv0')
        if args.cpu < 6:
            tryhop(2);POP()
        RET()

    module(name='rt_ldivworker.s',
           code=[ ('EXPORT', '__@ldivworker'),
                  ('IMPORT', '__@lsub_t0t1') if args.cpu < 6 else ('NOP',),
                  ('IMPORT', '__@lcmpu_t0t1') if args.cpu < 6 else ('NOP',),
                  ('CODE', '__@ldivworker', code_ldivworker) ])

    def code_ldivprep():
        nohop()
        label('__@ldivprep')
        STW(T3);DEEK();STW(T0);
        LDW(T3);ADDI(2);DEEK();STW(T0+2);
        ORW(T0);_BNE('.ldp1')
        _CALLJ('_@_raise_zdiv')
        label('.ldp1')
        if args.cpu >= 6:
            LDI(32)
        else:
            LDI(256-32)
        STW(T4)
        RET()

    module(name='rt_ldivprep.s',
           code=[ ('EXPORT', '__@ldivprep'),
                  ('IMPORT', '_@_raise_zdiv'),
                  ('CODE',   '__@ldivprep', code_ldivprep) ] )

    # ----------------------------------------
    # LDIVU: LAC / [vAC] -> LAC
    # ----------------------------------------

    def code_ldivu():
        """ Unsigned division LAC / [vAC] -> LAC.
            Thrashes T[0-5] sysArgs[0-7]."""
        label('_@_ldivu')
        PUSH()
        _CALLI('__@ldivprep')
        CallWorker()
        _MOVW(T2,LAC);_MOVW(T2+2,LAC+2)
        tryhop(2);POP();RET()

    module(name='rt_ldivu.s',
           code=[ ('EXPORT', '_@_ldivu'),
                  ('IMPORT', '__@ldivprep'),
                  ('CODE',   '_@_ldivu', code_ldivu),
                  ('IMPORT', '__@ldivworker') ] )


    # ----------------------------------------
    # LMODU: LAC%[vAC]->vAC, LAC/[vAC]->T0T1
    # ----------------------------------------

    def code_lmodu():
        """ Unsigned modulo LAC % [vAC] -> LAC.
            Returns quotient in T[01]
            Thrashes T[0-5] sysArgs[0-7]."""
        label('_@_lmodu')
        PUSH()
        _CALLI('__@ldivprep')
        CallWorker()
        LDW(T2);STW(T0);LDW(T2+2);STW(T0+2)
        tryhop(2);POP();RET()

    module(name='rt_lmodu.s',
           code=[ ('EXPORT', '_@_lmodu'),
                  ('IMPORT', '__@ldivprep'),
                  ('CODE',   '_@_lmodu', code_lmodu),
                  ('IMPORT', '__@ldivworker') ] )


    # ----------------------------------------

    def code_ldivsign():
        # Signed division helper
        # T4H bit 7 : quotient sign
        # T4H bit 6 : remainder sign
        label('__@ldivsign')
        PUSH()
        LDW(LAC+2);_BGE('.lds1')
        if args.cpu >= 6:
            NEGVL(LAC)
        else:
            _CALLJ('_@_lneg')
        LD(T4+1);XORI(0xc0);ST(T4+1)
        label('.lds1')
        LDW(T0+2);_BGE('.lds2')
        if args.cpu >= 6:
            NEGVL(T0)
        else:
            _CALLJ('__@lneg_t0t1')
        LD(T4+1);XORI(0x80);ST(T4+1)
        label('.lds2')
        tryhop(2);POP();RET()

    module(name='rt_ldivsign.s',
           code=[ ('EXPORT', '__@ldivsign'),
                  ('IMPORT', '__@lneg_t0t1') if args.cpu < 6 else ('NOP',),
                  ('IMPORT', '_@_lneg') if args.cpu < 6 else ('NOP',),
                  ('CODE',   '__@ldivsign', code_ldivsign) ] )

    # ----------------------------------------
    # LDIVS: LAC / [vAC] -> LAC
    # ----------------------------------------

    def code_ldivs():
        """ Signed division LAC / [vAC] -> LAC.
            Thrashes T[0-3] sysArgs[0-7]."""
        label('_@_ldivs')
        PUSH()
        _CALLI('__@ldivprep')
        _CALLJ('__@ldivsign')
        CallWorker()
        _MOVW(T2,LAC);_MOVW(T2+2,LAC+2)
        LDW(T4);_BGE('.ret')
        if args.cpu >= 6:
            NEGVL(LAC)
        else:
            _CALLJ('_@_lneg')
        label('.ret')
        tryhop(2);POP();RET()

    module(name='rt_ldivs.s',
           code=[ ('EXPORT', '_@_ldivs'),
                  ('IMPORT', '__@ldivprep'),
                  ('IMPORT', '__@ldivsign'),
                  ('IMPORT', '_@_lneg') if args.cpu < 6 else ('NOP',),
                  ('CODE',   '_@_ldivs', code_ldivs),
                  ('IMPORT', '__@ldivworker') ] )

    # ----------------------------------------
    # LMODS: LAC%[vAC]->vAC, LAC/[vAC]->T0T1
    # ----------------------------------------

    def code_lmods():
        """ Signed modulo LAC % [vAC] -> LAC.
            Returns quotient in T[01]
            Thrashes T[0-3] sysArgs[0-7]."""
        label('_@_lmods')
        PUSH()
        _CALLI('__@ldivprep')
        _CALLJ('__@ldivsign')
        CallWorker()
        _MOVW(T2,T0);_MOVW(T2+2,T0+2)
        LDW(T4);_BGE('.lms1')
        if args.cpu >= 6:
            NEGVL(T0)
        else:
            _CALLJ('__@lneg_t0t1')
        label('.lms1')
        LD(T4+1);ANDI(0x40);_BEQ('.lms2')
        if args.cpu >= 6:
            NEGVL(LAC)
        else:
            _CALLJ('_@_lneg')
        label('.lms2')
        tryhop(2);POP();RET()

    module(name='rt_lmods.s',
           code=[ ('EXPORT', '_@_lmods'),
                  ('IMPORT', '__@ldivprep'),
                  ('IMPORT', '__@ldivsign'),
                  ('IMPORT', '__@lneg_t0t1') if args.cpu < 6 else ('NOP',),
                  ('IMPORT', '_@_lneg') if args.cpu < 6 else ('NOP',),
                  ('CODE',   '_@_lmods', code_lmods),
                  ('IMPORT', '__@ldivworker') ] )

scope()

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
