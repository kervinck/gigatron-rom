
def scope():

    if False and 'has_SYS_notyetavailable' in rominfo:
        error("Not implemented")
        pass

    else:
        # Long division using vCPU
        #  LAC:  in: dividend  out: remainder
        #  T0T1: in: divisor
        #  T2T3: out: quotient
        # Clobbers B1

        def CallWorker():
            _CALLJ('__@ldivworker')

        def code_ldivworker():
            label('__@ldivworker')
            PUSH()
            LDW(LAC);STW(T2);LDW(LAC+2);STW(T2+2)
            LDI(0);ST(B1);STW(LAC);STW(LAC+2)
            label('.ldiv0')
            if args.cpu >= 6:
                LDI(0);NROL(4,T2);NROL(4,LAC)
            else:
                LDW(LAC+2);LSLW();STW(LAC+2)
                LDW(LAC);_BGE('.ldw1')
                LDI(1);ORW(LAC+2);STW(LAC+2);LDW(LAC)
                label('.ldw1');LSLW();STW(LAC)
                LDW(T2+2);_BGE('.ldw2')
                LDI(1);ORW(LAC);STW(LAC);LDW(T2+2)
                label('.ldw2');LSLW();STW(T2+2)
                LDW(T2);_BGE('.ldw3')
                LDI(1);ORW(T2+2);STW(T2+2);LDW(T2)
                label('.ldw3');LSLW();STW(T2)
            
            if args.cpu >= 6:
                LDI(T0);CMPLPU();BLT('.ldiv1')
                LDI(T0);SUBLP();INC(T2)
            else:
                _CALLJ('__@lcmpu_t0t1');_BLT('.ldiv1')
                _CALLJ('__@lsub_t0t1');INC(T2)
            label('.ldiv1')
            INC(B1);LD(B1);XORI(32);_BNE('.ldiv0')
            tryhop(2);POP();RET()

        module(name='rt_ldivworker.s',
               code=[ ('EXPORT', '__@ldivworker'),
                      ('IMPORT', '__@lsub_t0t1') if args.cpu < 6 else ('NOP',),
                      ('IMPORT', '__@lcmpu_t0t1') if args.cpu < 6 else ('NOP',),
                      ('CODE', '__@ldivworker', code_ldivworker) ])

        morecode = [('IMPORT', '__@ldivworker')]


    def code_ldivprep():
        nohop()
        label('__@ldivprep')
        STW(T3);DEEK();STW(T0);
        LDW(T3);ADDI(2);DEEK();STW(T0+2);
        ORW(T0);_BNE('.ldp1')
        _CALLJ('_@_raise_zdiv')
        label('.ldp1')
        RET()

    module(name='rt_ldivprep.s',
           code=[ ('EXPORT', '__@ldivprep'),
                  ('IMPORT', '_@_raise_zdiv'),
                  ('CODE',   '__@ldivprep', code_ldivprep) ] )

    def code_ldivu():
        # LDIVU : LAC <- LAC / [vAC]
        label('_@_ldivu')
        PUSH()
        _CALLI('__@ldivprep')
        CallWorker()
        LDW(T2);STW(LAC);LDW(T2+2);STW(LAC+2)
        tryhop(2);POP();RET()

    module(name='rt_ldivu.s',
           code=[ ('EXPORT', '_@_ldivu'),
                  ('IMPORT', '__@ldivprep'),
                  ('CODE',   '_@_ldivu', code_ldivu) ] + morecode )

    def code_lmodu():
        # LMODU : LAC <- LAC % [vAC]
        #        T0T1 <- LAC / [vAC]
        label('_@_lmodu')
        PUSH()
        _CALLI('__@ldivprep')
        CallWorker()
        LDW(T2);STW(T0);LDW(T2+2);STW(T0+2)
        tryhop(2);POP();RET()

    module(name='rt_lmodu.s',
           code=[ ('EXPORT', '_@_lmodu'),
                  ('IMPORT', '__@ldivprep'),
                  ('CODE',   '_@_lmodu', code_lmodu) ] + morecode )

    def code_ldivsign():
        # B2 bit 7 : quotient sign
        # B2 bit 1 : remainder sign
        label('__@ldivsign')
        PUSH()
        LDI(0);ST(B2)
        LDW(LAC+2);_BGE('.lds1')
        if args.cpu >= 6:
            NEGL(LAC)
        else:
            _CALLJ('_@_lneg')
        LD(B2);XORI(0x81);ST(B2)
        label('.lds1')
        LDW(T0+2);_BGE('.lds2')
        if args.cpu >= 6:
            NEGL(T0)
        else:
            _CALLJ('__@lneg_t0t1')
        LD(B2);XORI(0x80);ST(B2)
        label('.lds2')
        tryhop(2);POP();RET()

    module(name='rt_ldivsign.s',
           code=[ ('EXPORT', '__@ldivsign'),
                  ('IMPORT', '__@lneg_t0t1') if args.cpu < 6 else ('NOP',),
                  ('IMPORT', '_@_lneg') if args.cpu < 6 else ('NOP',),
                  ('CODE',   '__@ldivsign', code_ldivsign) ] )

    def code_ldivs():
        # LDIVS : LAC <- LAC / [vAC]
        label('_@_ldivs')
        PUSH()
        _CALLI('__@ldivprep')
        _CALLJ('__@ldivsign')
        CallWorker()
        LDW(T2);STW(LAC);LDW(T2+2);STW(LAC+2)
        LD(B2);ANDI(0x80);_BEQ('.ret')
        if args.cpu >= 6:
            NEGL(LAC)
        else:
            _CALLJ('_@_lneg')
        label('.ret')
        tryhop(2);POP();RET()

    module(name='rt_ldivs.s',
           code=[ ('EXPORT', '_@_ldivs'),
                  ('IMPORT', '__@ldivprep'),
                  ('IMPORT', '__@ldivsign'),
                  ('IMPORT', '_@_lneg') if args.cpu < 6 else ('NOP',),
                  ('CODE',   '_@_ldivs', code_ldivs) ] + morecode )

    def code_lmods():
        # LMODS : LAC <- LAC % [vAC]
        #        T0T1 <- LAC / [vAC]
        label('_@_lmods')
        PUSH()
        _CALLI('__@ldivprep')
        _CALLJ('__@ldivsign')
        CallWorker()
        LDW(T2);STW(T0);LDW(T2+2);STW(T0+2)
        LD(B2);ANDI(0x80);_BEQ('.lms1')
        if args.cpu >= 6:
            NEGL(T0)
        else:
            _CALLJ('__@lneg_t0t1')
        label('.lms1')
        LD(B2);ANDI(0x01);BEQ('.lms2')
        if args.cpu >= 6:
            NEGL(LAC)
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
                  ('CODE',   '_@_lmods', code_lmods) ] + morecode )

scope()

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
