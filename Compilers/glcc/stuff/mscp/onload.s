
def scope():

    def code1():
        nohop()
        label('mscp_onload')
        # copy 0xc000-0xffff into bank 3
        PUSH()
        LDWI('SYS_LSRW2_52');STW('sysFn')
        LDI(0);STW(R8)
        LDWI('ctrlBits_v5');PEEK();SYS(52);ANDI(0x30);ORI(0xc0);ST(R8+1)
        LDWI('SYS_CopyMemoryExt_v6_100');STW('sysFn')
        LDWI(0xc000);STW(R9)
        label('.loop')
        STW('sysArgs0');STW('sysArgs2')
        LDW(R8);SYS(100)
        INC(R9+1);LDW(R9)
        BNE('.loop')
        # set prebooksize
        LDWI('prebooksize');STW(T2)
        LDWI(0xbffe);DEEK();DOKE(T2)
        POP()
        RET()

    module(name='onload.s',
       code=[ ('EXPORT', 'mscp_onload'),
              ('IMPORT', 'prebooksize'),
              ('CODE', 'mscp_onload', code1) ] )

scope()

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
