
def scope():

    def code0():
        nohop()
        label('printf')
        # calling _sim_flush if already present
        _LDI('__glink_weak__sim_flush');_BEQ('.run')
        LDW(vLR);DOKE(SP);_LDI(-12);ADDW(SP);STW(SP)
        _SP(12-4);STW(R22);LDW(R8);DOKE(R22)
        _CALLJ('__glink_weak__sim_flush')
        _SP(12-4);DEEK();STW(R8)
        _LDI(12);ADDW(SP);STW(SP);DEEK();STW(vLR)
        # done
        label('.run')
        LDWI(0xff01);STW('sysFn');SYS(34)
        RET();

    module(name='printf.s',
           code=[ ('EXPORT', 'printf'),
                  ('CODE', 'printf', code0) ] )

scope()

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
