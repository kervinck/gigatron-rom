def scope():

    def _MOVQW(i,d):
        if args.cpu >= 6:
            MOVQW(i,d)
        else:
            LDI(i);STW(d)

    if 'has_at67_SYS_Divide_s16' in rominfo:
        # Divide using SYS call
        XV = v('sysArgs0'); YV = v('sysArgs2')
        RV = v('sysArgs4'); MV = v('sysArgs6')

        info = rominfo['has_at67_SYS_Divide_s16']
        addr = int(str(info['addr']),0)
        cycs = int(str(info['cycs']),0)
        def CallWorker():
            _MOVQW(0,RV)
            _MOVQW(1,MV)
            _LDI(addr);STW('sysFn');SYS(cycs)

        morecode = []

    else:
        # Divide using vCPU
        XV = T3; YV = T2;
        RV = T0; MV = T1

        def CallWorker():
            _CALLJ('__@divworker')

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

        morecode = [('IMPORT', '__@divworker')]

    # DIVU:  T3/vAC -> vAC
    assert YV != T3
    def code2():
        label('_@_divu')
        PUSH()
        STW(YV);_BLT('.bigy');_BNE('.divu1')
        _CALLJ('_@_raise_zdiv')                    # divide by zero error (no return)
        label('.divu1')
        LDW(T3)
        if XV != T3: STW(XV)
        _BLT('.bigx');
        label('.divu2')
        CallWorker()
        LDW(XV)
        tryhop(2);POP();RET()
        # special cases
        label('.bigx')                             # x >= 0x8000
        LD(YV+1);ANDI(0x40);_BEQ('.divu2')         # - but y is small enough
        label('.bigy')                             # y large
        _MOVQW(0,MV)                               # - repeated subtractions
        LDW(T3)
        if XV != T3: STW(XV)
        BRA('.loop1')
        label('.loop0')                            #   (loops at most 3 times)
        INC(MV)
        LDW(XV);SUBW(YV);STW(XV)                   #   (warning: SUBVW overwrites MV!)
        label('.loop1')
        _CMPWU(YV);_BGE('.loop0')
        LDW(XV);STW(RV)                            # - for modu
        LDW(MV)
        tryhop(2);POP();RET()

    module(name='rt_divu.s',
           code=[ ('CODE', '_@_divu', code2),
                  ('IMPORT', '_@_raise_zdiv'),
                  ('EXPORT', '_@_divu') ] + morecode )

    # MODU: T3 % T2 -> AC
    #  saves T3 / T2 in T1
    assert(RV != T1)
    def code2():
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

    # DIVS:  T3/vAC -> vAC
    # clobbers B2
    assert YV != T3
    def code2():
        label('_@_divs')
        PUSH()
        if args.cpu >= 6:
            MOVQB(0,B2)
            _BGT('.divs1');_BNE('.divs0')
            _CALLJ('_@_raise_zdiv')
            label('.divs0')
            NEGW(vAC);INC(B2)
            label('.divs1')
            STW(YV)
            LDW(T3);_BGE('.divs2')
            NEGW(vAC);XORBI(3,B2)
        else:
            STW(YV);
            LDI(0);ST(B2)
            LDW(YV);_BGT('.divs1');_BNE('.divs0')
            _CALLJ('_@_raise_zdiv')
            label('.divs0')
            LDI(0);SUBW(YV);STW(YV);INC(B2)
            label('.divs1')
            LDW(T3);_BGE('.divs2')
            LD(B2);XORI(3);ST(B2)
            LDI(0);SUBW(T3)
        label('.divs2')
        STW(XV)
        CallWorker()
        LD(B2);ANDI(1);_BEQ('.divs4')
        if args.cpu >= 6:
            NEGW(XV)
        else:
            LDI(0);SUBW(XV)
            tryhop(2);POP();RET()
        label('.divs4')
        LDW(XV)
        tryhop(2);POP();RET()

    module(name='rt_divs.s',
           code=[ ('CODE', '_@_divs', code2),
                  ('IMPORT', '_@_raise_zdiv'),
                  ('EXPORT', '_@_divs') ] + morecode )

    # MODS: T3 % T2 -> AC
    #  saves T3 / T2 in T1
    #  clobbers B2
    assert RV != T1
    def code2():
        label('_@_mods')
        PUSH()
        _CALLI('_@_divs')
        STW(T1)               # quotient
        LD(B2);ANDI(2);_BEQ('.mods1')
        if args.cpu >= 6:
            NEGW(RV)
        else:
            LDI(0);SUBW(RV)
            tryhop(2);POP();RET()
        label('.mods1')
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
