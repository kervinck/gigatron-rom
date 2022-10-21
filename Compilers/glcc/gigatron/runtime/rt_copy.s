def scope():

    # LCOPY [T0..T0+3] --> [T2..]
    # Since longs are even aligned,
    # we cannot cross a page boundary inside the DEEK/DOKE

    def code1():
        nohop()
        label('_@_lcopy_')
        if args.cpu >= 6:
            ## With cpu>=6, the macro _MOVL does not call this function
            ## but uses opcodes MOVL, DEEKL, DOKEL, or NCOPY instead.
            DEEKV(T0);DOKE(T2)
            ADDVI(2,T0);ADDVI(2,T2)
            DEEKV(T0);DOKE(T2)
        else:
            LDW(T0);DEEK();DOKE(T2)
            LDI(2);ADDW(T2);STW(T2)
            LDI(2);ADDW(T0);DEEK();DOKE(T2)
        RET()

    module(name='rt_lcopy.s',
           code=[ ('EXPORT', '_@_lcopy_'),
                  ('CODE', '_@_lcopy_', code1) ] )

    # FCOPYZ LCOPYZ: Zero page copy for floats and longs
    #   with short call sequence:  LDWI(<dst><src>);CALLI
    # FCOPYNC: Float copy when no page crossings
    def code2():
        nohop()
        if args.cpu >= 6:
            ## With cpu>=6, the macros _MOVF and _MOVL do not call this
            ## function but use opcodes MOVF, MOVL, or NCOPY instead.
            label('_@_fcopyz_')
            ST(T0);LD(vACH);STW(T2)
            LD(T0);STW(T0)
            label('_@_fcopync_')
            PEEKp(T0);POKEp(T2)
            BRA('.cont')
            label('_@_lcopyz_')
            ST(T0);LD(vACH);STW(T2)
            MOVQB(0, T0+1)
            label('.cont')
            DEEKp(T0);DOKEp(T2)
            DEEKp(T0);DOKEp(T2)
            RET()
        else:
            label('_@_fcopyz_')
            ST(T0);LD(vACH);STW(T2)
            LD(T0);STW(T0);
            label('_@_fcopync_')
            LDW(T0);PEEK();POKE(T2)
            INC(T2);INC(T0);LDW(T0)
            BRA('.cont')
            label('_@_lcopyz_')
            ST(T0);LD(vACH);STW(T2)
            LD(T0);STW(T0)
            label('.cont')
            DEEK();DOKE(T2)
            INC(T2);INC(T0)
            INC(T2);INC(T0)
            LDW(T0);DEEK();DOKE(T2)
            RET()

    module(name='rt_copyz.s',
           code=[ ('EXPORT', '_@_lcopyz_'),
                  ('EXPORT', '_@_fcopyz_'),
                  ('EXPORT', '_@_fcopync_'),
                  ('CODE', '_@_fcopyz_', code2) ])


    # FCOPY [T0..T0+5) --> [T2..T2+5)
    # BCOPY [T0..T1) --> [T2..]
    # When we can rely on nothing.
    def code3():
        nohop()
        label('_@_fcopy_')
        LDI(5);ADDW(T0);STW(T1)
        label('_@_bcopy_')
        _PEEKV(T0);POKE(T2)
        if args.cpu >= 6:
            ## With cpu>=6, the macro _MOVF and _MOVM do not call this
            ## function but use opcode NCOPY instead.
            INCW(T2);INCW(T0);LDW(T0)
        else:
            LDI(1);ADDW(T2);STW(T2)
            LDI(1);ADDW(T0);STW(T0)
        XORW(T1);_BNE('_@_bcopy_')
        RET()

    module(name='rt_bcopy.s',
           code=[ ('EXPORT', '_@_bcopy_'),
                  ('EXPORT', '_@_fcopy_'),
                  ('CODE', '_@_bcopy_', code3) ])

    # WCOPY [T0..T1) --> [T2..]
    # Same as BCOPY but word aligned

    def code4():
        nohop()
        label('_@_wcopy_')
        _DEEKV(T0);DOKE(T2)
        if args.cpu >= 6:
            ## With cpu>=6, the macro _MOVF and _MOVM do not call this
            ## function but use opcode NCOPY instead.
            ADDVI(2,T2);ADDVI(2,T0)
        else:
            LDI(2);ADDW(T2);STW(T2)
            LDI(2);ADDW(T0);STW(T0)
        XORW(T1);_BNE('_@_wcopy_')
        RET()

    module(name='rt_wcopy.s',
           code=[ ('EXPORT', '_@_wcopy_'),
                  ('CODE', '_@_wcopy_', code4) ])

    # LEXTS: (vAC<0) ? -1 : 0 --> vAC
    # Cpu6 uses opcode STLS instead
    def code5():
        nohop()
        label('_@_lexts')
        _BLT('.m1')
        LDI(0);RET();
        label('.m1')
        _LDI(-1);RET()

    module(name='rt_lexts.s',
           code=[ ('EXPORT', '_@_lexts'),
                  ('CODE', '_@_lexts', code5) ])

    # LCVI: AC to LAC with sign extension
    # Cpu6 uses opcode STLS instead
    def code6():
        nohop()
        label('_@_lcvi')
        STW(LAC);
        LD(vACH);XORI(128);SUBI(128)
        LD(vACH);ST(LAC+2);ST(LAC+3)
        RET()

    module(name='rt_lcvi.s',
           code=[ ('EXPORT', '_@_lcvi'),
                  ('CODE', '_@_lcvi', code6) ])

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
