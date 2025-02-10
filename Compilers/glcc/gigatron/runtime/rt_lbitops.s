def scope():

    def code0():
        nohop()
        label('_@_lcom')
        if args.cpu >= 6:
            NOTVL(LAC)
            warning("cpu6: should use NOTVL instead of _@_lcom")
        else:
            _LDI(0xffff);XORW(LAC);STW(LAC)
            _LDI(0xffff);XORW(LAC+2);STW(LAC+2)
        RET()

    module(name='rt_lcom.s',
           code=[ ('EXPORT', '_@_lcom'),
                  ('CODE', '_@_lcom', code0) ])

    def code1():
        nohop()
        label('_@_land')
        if args.cpu >= 6:
            ANDL()
            warning("cpu6: should use ANDL instead of _@_land")
        else:
            STW(T3);DEEK();ANDW(LAC);STW(LAC)
            LDI(2);ADDW(T3);DEEK();ANDW(LAC+2);STW(LAC+2)
        RET()

    module(name='rt_land.s',
           code=[ ('EXPORT', '_@_land'),
                  ('CODE', '_@_land', code1) ])

    def code2():
        nohop()
        label('_@_lor')
        if args.cpu >= 6:
            ORL()
            warning("cpu6: should use ORL instead of _@_lor")
        else:
            STW(T3);DEEK();ORW(LAC);STW(LAC)
            LDI(2);ADDW(T3);DEEK();ORW(LAC+2);STW(LAC+2)
        RET()

    module(name='rt_lor.s',
           code=[ ('EXPORT', '_@_lor'),
                  ('CODE', '_@_lor', code2) ])

    def code3():
        nohop()
        label('_@_lxor')
        if args.cpu >= 6:
            XORL()
            warning("cpu6: should use XORL instead of _@_lxor")
        else:
            STW(T3);DEEK();XORW(LAC);STW(LAC)
            LDI(2);ADDW(T3);DEEK();XORW(LAC+2);STW(LAC+2)
        RET()

    module(name='rt_lxor.s',
           code=[ ('EXPORT', '_@_lxor'),
                  ('CODE', '_@_lxor', code3) ])

scope()

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
