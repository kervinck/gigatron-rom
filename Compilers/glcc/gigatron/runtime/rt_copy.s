def scope():

    # LCOPY [T3..T3+3] --> [T2..]
    # -- Copy an even-aligned long. Trashes T[1-3].
    #    The COPYN alternative also uses sysArgs[2-7].

    def code1():
        nohop()
        if args.cpu >= 6:
            warning("cpu6: use COPYN instead of calling _@_lcopy")
        label('_@_lcopy')
        LDW(T3);DEEK();DOKE(T2)
        LDI(2);ADDW(T2);STW(T2)
        LDI(2);ADDW(T3);DEEK();DOKE(T2)
        RET()

    module(name='rt_lcopy.s',
           code=[ ('EXPORT', '_@_lcopy'),
                  ('CODE', '_@_lcopy', code1) ] )

    # FCOPYZ LCOPYZ: [vACL..] --> [vACH..]
    # -- Zero page copy for floats and longs withing page zero
    #    with a short call sequence LDWI(<dst><src>);CALLI(..)
    #    Trashes T[2-3]. The MOVF/MOVL alternatives use sysArgs
    #    but we use T[2-3] to share code with FCOPYNC.

    # FCOPYNC: [T3..T3+4] --> [T2..]
    # -- Float copy when no page crossings. Trashes T[2-3].
    #    The COPYN alternatives also trashes sysArgs[2-7].

    def code2():
        nohop()
        if args.cpu >= 6:
            warning("cpu6: use MOVL/MOVF instead of _@_fcopyz/_@_lcopyz")
        label('_@_fcopync')
        LDW(T3)
        BRA('.cont5')
        label('_@_fcopyz')
        ST(T3);LD(vACH);STW(T2)
        LD(T3);STW(T3)
        label('.cont5')
        PEEK();POKE(T2)
        INC(T2);INC(T3);LDW(T3)
        BRA('.cont4')
        label('_@_lcopyz')
        ST(T3);LD(vACH);STW(T2)
        LD(T3);STW(T3)
        label('.cont4')
        DEEK();DOKE(T2)
        INC(T2);INC(T2)
        INC(T3);INC(T3);LDW(T3)
        DEEK();DOKE(T2)
        RET()

    module(name='rt_copyz.s',
           code=[ ('EXPORT', '_@_lcopyz'),
                  ('EXPORT', '_@_fcopyz'),
                  ('EXPORT', '_@_fcopync'),
                  ('CODE', '_@_fcopyz', code2) ])


    # FCOPY [T3..T3+5) --> [T2..T2+5)
    # BCOPY [T3..T1) --> [T2..]
    # -- Copy a float or a structure byte per byte. Trashes T[1-3].
    #    The COPYN alternative also trashes sysArgs[2-7]

    def code3():
        nohop()
        if args.cpu >= 6:
            warning("cpu6: use COPYN instead of _@_fcopy/_@_bcopy")
        label('_@_fcopy')
        LDI(5);ADDW(T3);STW(T1)
        label('_@_bcopy')
        _PEEKV(T3);POKE(T2)
        LDI(1);ADDW(T2);STW(T2)
        LDI(1);ADDW(T3);STW(T3)
        XORW(T1);_BNE('_@_bcopy')
        RET()

    module(name='rt_bcopy.s',
           code=[ ('EXPORT', '_@_bcopy'),
                  ('EXPORT', '_@_fcopy'),
                  ('CODE', '_@_bcopy', code3) ])

    # WCOPY [T3..T1) --> [T2..]
    # -- Copy a structure word by word. Trashes T[1-3].
    #    The COPYN alternative also trashes sysArgs[2-7]
    def code4():
        nohop()
        if args.cpu >= 6:
            warning("cpu6: use COPYN instead of _@_wcopy")
        label('_@_wcopy')
        _DEEKV(T3);DOKE(T2)
        LDI(2);ADDW(T2);STW(T2)
        LDI(2);ADDW(T3);STW(T3)
        XORW(T1);_BNE('_@_wcopy')
        RET()

    module(name='rt_wcopy.s',
           code=[ ('EXPORT', '_@_wcopy'),
                  ('CODE', '_@_wcopy', code4) ])

    # LEXTS: (vAC<0) ? -1 : 0 --> vAC
    def code5():
        nohop()
        label('_@_lexts')
        if args.cpu >= 7:
            LDSB(vACH);LDSB(vACH)
            RET()
        else:
            _BLT('.m1')
            LDI(0);RET();
            label('.m1')
            _LDI(-1);RET()

    module(name='rt_lexts.s',
           code=[ ('EXPORT', '_@_lexts'),
                  ('CODE', '_@_lexts', code5) ])

    # The following are merely markers indicating that _MOVL or _MOVF
    # is used somewhere. These are useful to decide whether to import
    # printf support for longs or floats.

    def code_dummy():
        label('_@_using_lmov', 1)

    module(name='rt_lmov',
           code=[('EXPORT', '_@_using_lmov'),
                 ('DATA', '_@_using_lmov', code_dummy, 0, 1) ] )

    def code_dummy():
        label('_@_using_fmov', 1)

    module(name='rt_fmov',
           code=[('EXPORT', '_@_using_fmov'),
                 ('DATA', '_@_using_fmov', code_dummy, 0, 1) ] )

scope()

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
