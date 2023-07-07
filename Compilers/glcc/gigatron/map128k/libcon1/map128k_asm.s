

def scope():

    screenStart=130
    screenEnd=screenStart+120

    def code0():
        nohop()
        label('_con128ksetup')
        LDWI('videoTable');STW(R8)
        LDI(screenStart);STW(R9)
        label('.c128loop')
        LDW(R9);DOKE(R8)
        LDI(2);ADDW(R8);STW(R8)
        LDI(1);ADDW(R9);STW(R9)
        XORI(screenEnd);BNE('.c128loop')
        RET()
        
    def code1():
        nohop()
        label('_map128ksetup')
        # copy gt1 data into bank2
        PUSH()
        LDWI('SYS_LSRW2_52');STW('sysFn')
        LDI(0);STW(R8)
        LDWI('ctrlBits_v5');PEEK();SYS(52);ANDI(0x30);ORI(0x80);ST(R8+1)
        LDWI('SYS_CopyMemoryExt_v6_100');STW('sysFn')
        LDWI('_egt1');DEEK();SUBI(1);ORI(255);XORI(255);STW(R9)
        _BRA('.m128copytest')
        label('.m128copyloop')
        STW('sysArgs0');STW('sysArgs2')
        LDW(R8);SYS(100)
        LDWI(-256);ADDW(R9);STW(R9)
        label('.m128copytest')
        BLT('.m128copyloop')
        POP()
        LDWI('SYS_ExpanderControl_v4_40');STW('sysFn')
        LDWI('ctrlBits_v5');PEEK();ANDI(0x3c);ORI(0x80);SYS(40)
        PUSH()
        CALLI('_con128ksetup')
        LDWI(screenStart << 8);STW(R8)
        LDI(0);STW(R9)
        LDI(120);STW(R10)
        CALLI('_console_clear')
        tryhop(2);POP();RET()
    
    module(name='map128ksetup.s',
           code=[ ('EXPORT', '_map128ksetup'),
                  ('EXPORT', '_con128ksetup'),
                  ('IMPORT', '_egt1'),
                  ('IMPORT', '_console_clear'),
                  ('CODE', '_con128ksetup', code0),
                  ('CODE', '_map128ksetup', code1),
                  ('PLACE', '_map128ksetup', 0x0200, 0x7fff) ] )
    
scope()

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
