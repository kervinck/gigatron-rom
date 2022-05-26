
def scope():

    def savemask(i):
        return (0xff << i) & 0xff
    def savename(i):
        return "_@_save_%02x" % savemask(i)
    def rtrnname(i):
        return "_@_rtrn_%02x" % savemask(i)

    def code0():
        nohop()
        for i in range(0,8):
            label(savename(i))
            if args.cpu >= 6:
                DOKEA(R0+i+i)
                if i < 7: ADDI(2) 
            elif args.cpu >= 5:
                STW(T3);LDW(R0+i+i);DOKE(T3)
                if i < 7: LDI(2);ADDW(T3)
            else:
                LDW(R0+i+i);DOKE(T3)
                if i < 7: LDI(2);ADDW(T3);STW(T3)
        RET()

    def code1():
        nohop()
        for i in range(0,8):
            label(rtrnname(i))
            if args.cpu >= 6:
                DEEKA(R0+i+i)
                if i < 7: ADDI(2)
            elif args.cpu >= 5:
                STW(T3);DEEK();STW(R0+i+i)
                if i < 7: LDI(2);ADDW(T3)
            else:
                LDW(T3);DEEK();STW(R0+i+i)
                if i < 7: LDI(2);ADDW(T3);STW(T3)
        label(rtrnname(8))
        if args.cpu >= 6:
            LDW(SP);DEEKA(vLR)
        else:
            LDW(SP);DEEK();STW(vLR)
        LDW(T2);RET()

    module(name='rt_save.s', code=
           [ ('EXPORT', savename(i)) for i in range(0,8) ] + \
           [ ('EXPORT', rtrnname(i)) for i in range(0,9) ] + \
           [ ('CODE', savename(0), code0),
             ('CODE', rtrnname(0), code1) ] )

scope()

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
