
def scope():

    def savemask(i):
        return (0xff << i) & 0xff
    def savename(i):
        return "_@_save_%02x" % savemask(i)
    def rtrnname(i):
        return "_@_rtrn_%02x" % savemask(i)

    def code0():
        nohop()
        if args.cpu >= 6:
            for i in range(0,8):
                label(savename(i))
                DOKEA(R0+i+i);ADDI(2)
            label(savename(8))
            DOKEA(B0)
        elif args.cpu >= 5:
            for i in range(0,8):
                label(savename(i))
                STW(T2);LDW(R0+i+i);DOKE(T2)
                LDI(2);ADDW(T2)
            label(savename(8))
            STW(T2);LDW(B0);DOKE(T2)
        else:
            for i in range(0,8):
                label(savename(i))
                LDW(R0+i+i);DOKE(T2)
                LDI(2);ADDW(T2);STW(T2)
            label(savename(8))
            LDW(B0);DOKE(T2)
        RET()

    def code1():
        nohop()
        if args.cpu >= 6:
            for i in range(0,8):
                label(rtrnname(i))
                DEEKA(R0+i+i);ADDI(2)
            label(rtrnname(8))
            DEEKA(vLR)
        elif args.cpu >= 5:
            for i in range(0,8):
                label(rtrnname(i))
                STW(T3);DEEK();STW(R0+i+i)
                LDI(2);ADDW(T3)
            label(rtrnname(8))
            STW(T3);DEEK();STW(vLR)
        else:
            for i in range(0,8):
                label(rtrnname(i))
                LDW(T3);DEEK();STW(R0+i+i)
                LDI(2);ADDW(T3);STW(T3)
            label(rtrnname(8))
            LDW(T3);DEEK();STW(vLR)
        # return
        LDW(R8);RET()

    module(name='rt_save.s', code=
           [ ('EXPORT', savename(i)) for i in range(0,9) ] + \
           [ ('EXPORT', rtrnname(i)) for i in range(0,9) ] + \
           [ ('CODE', savename(0), code0),
             ('CODE', rtrnname(0), code1) ] )

scope()

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
