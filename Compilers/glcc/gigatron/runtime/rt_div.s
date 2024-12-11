def scope():

    XV = v('sysArgs0'); YV = v('sysArgs2')
    RV = v('sysArgs4'); MV = v('sysArgs6')
    assert YV != T3
    assert XV != T3

    if 'has_SYS_Divide_u16' in rominfo:
        # Divide using SYS call
        morecode = []
        needbig = False
        info = rominfo['has_SYS_Divide_u16']
        addr = int(str(info['addr']),0)
        cycs = int(str(info['cycs']),0)
        def CallWorker():
            _LDI(addr);STW('sysFn');SYS(cycs)

    elif 'has_at67_SYS_Divide_s16' in rominfo:
        # Divide using SYS call
        morecode = []
        needbig = True
        info = rominfo['has_at67_SYS_Divide_s16']
        addr = int(str(info['addr']),0)
        cycs = int(str(info['cycs']),0)
        def CallWorker():
            LDI(0);STW(RV)
            LDI(1);STW(MV)
            _LDI(addr);STW('sysFn');SYS(cycs)

    elif args.cpu < 7:
        # Divide using vCPU
        needbig = True
        morecode = [('IMPORT', '__@divworker')]
        def code0():
            nohop()
            label('__@divworker')
            LDI(0);STW(RV);ST(MV)
            label('.div0')
            LDW(XV);_BGE('.div1')
            LSLW();STW(XV);
            LDW(RV);LSLW();ORI(1);_BRA('.div2')
            label('.div1')
            LSLW();STW(XV);
            LDW(RV);LSLW()
            label('.div2')
            STW(RV);SUBW(YV);_BLT('.div3')
            STW(RV);INC(XV)
            label('.div3')
            INC(MV);LD(MV);XORI(16);_BNE('.div0')
            RET()
        module(name='rt_divworker.s',
               code=[ ('CODE', '__@divworker', code0),
                      ('EXPORT', '__@divworker') ] )
        def CallWorker():
            _CALLJ('__@divworker')


    # ----------------------------------------
    # DIVU: T3/vAC -> vAC
    # ----------------------------------------

    def code2():
        """ Computes unsigned T3/vAC into vAC.
            May raise zdiv signal.
            Trashes sysArgs[0-7], T[013]. """
        label('_@_divu')
        PUSH()
        _BNE('.divu1')
        _CALLJ('_@_raise_zdiv')  # divide by zero error
        label('.divu1')
        if args.cpu >= 7:
            RDIVU(T3)
        else:
            STW(YV)
            if needbig:
                _BLT('.bigy')
            LDW(T3);STW(XV)
            if needbig:
                _BLT('.bigx')
            label('.divu2')
            CallWorker()
            if needbig:
                LDW(XV)
        tryhop(2);POP();RET()
        # special cases
        if needbig:
            label('.bigx')                          # x >= 0x8000
            LD(YV+1);ANDI(0x40);_BEQ('.divu2')      # - but y is small enough
            label('.bigy')                          # y large
            LDI(0);STW(MV)                          # - repeated subtractions
            assert MV != T3
            LDW(T3);STW(XV)
            BRA('.loop1')
            label('.loop0')                         #   (loops at most 3 times)
            INC(MV)
            LDW(XV);SUBW(YV);STW(XV)
            label('.loop1')
            _CMPWU(YV);_BGE('.loop0')
            LDW(XV);STW(RV)                         # - for modu
            LD(MV)
            tryhop(2);POP();RET()

    module(name='rt_divu.s',
           code=[ ('CODE', '_@_divu', code2),
                  ('IMPORT', '_@_raise_zdiv'),
                  ('EXPORT', '_@_divu') ] + morecode )


    # ----------------------------------------
    # MODU: T3 % vAC -> vAC, T3 / vAC in T1
    # ----------------------------------------

    assert(RV != T1)

    def code2():
        """ Computes unsigned T3%vAC into vAC. Saves quotient in T1
            May raise zdiv signal. Trashes sysArgs[0-7], T[013]. """
        label('_@_modu')
        PUSH()
        _CALLI('_@_divu')
        STW(T1)         # quotient
        LDW(RV)         # remainder
        tryhop(2);POP();RET()

    module(name='rt_modu.s',
           code=[ ('CODE', '_@_modu', code2),
                  ('EXPORT', '_@_modu'),
                  ('IMPORT', '_@_divu') ] )

    # ----------------------------------------
    # DIVS:  T3/vAC -> vAC
    # ----------------------------------------

    assert YV != T3

    def code2():
        """ Computes signed T3/vAC into vAC.
            May raise zdiv signal. Trashes sysArgs[0-7], T[0123]. """
        label('_@_divs')
        PUSH()
        _BNE('.divs0')
        _CALLJ('_@_raise_zdiv') # divide by zero error (no return)
        label('.divs0')
        if args.cpu >= 7:
            RDIVS(T3)
            tryhop(2);POP();RET()
        else:
            STW(YV)
            if args.cpu >= 6:
                MOVQB(0,T2)
                _BGT('.divs1')
                NEGV(YV);INC(T2)
                label('.divs1')
                LDW(T3);STW(XV);_BGE('.divs2')
                NEGV(XV)
            else:
                LDI(0);ST(T2)
                LDW(YV);_BGT('.divs1')
                LDI(0);SUBW(YV);STW(YV);INC(T2)
                label('.divs1')
                LDW(T3);STW(XV);_BGE('.divs2')
                LDI(0);SUBW(T3);STW(XV)
            LD(T2);XORI(3);ST(T2)
            label('.divs2')
            CallWorker()
            LD(T2);ANDI(2);_BEQ('.divs3')
            if args.cpu >= 6:
                NEGV(RV)
            else:
                LDI(0);SUBW(RV);STW(RV)
            label('.divs3')
            LD(T2);ANDI(1);_BEQ('.divs4')
            if args.cpu >= 6:
                NEGV(XV)
            else:
                LDI(0);SUBW(XV);STW(XV)
            label('.divs4')
            LDW(XV)
            tryhop(2);POP();RET()

    module(name='rt_divs.s',
           code=[ ('CODE', '_@_divs', code2),
                  ('IMPORT', '_@_raise_zdiv'),
                  ('EXPORT', '_@_divs') ] + morecode )

    # ----------------------------------------
    # MODS: T3 % vAC -> AC  T3 / vAC -> T1
    # ----------------------------------------

    assert RV != T1

    def code2():
        label('_@_mods')
        """ Computes signed T3%vAC into vAC. Saves quotient in T1
            May raise zdiv signal. Trashes sysArgs[0-7], T[0123]. """
        PUSH()
        _CALLI('_@_divs')
        STW(T1)               # quotient
        LDW(RV)               # remainder
        tryhop(2);POP();RET()

    module(name='rt_mods.s',
           code=[ ('CODE', '_@_mods', code2),
                  ('EXPORT', '_@_mods'),
                  ('IMPORT', '_@_divs') ] )

scope()

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
