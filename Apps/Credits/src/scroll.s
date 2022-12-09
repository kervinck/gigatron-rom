



def scope():

    if not 'has_SYS_CopyMemory' in rominfo:
        error("This code needs SYS_CopyMemory")
    info = rominfo['has_SYS_CopyMemory']
    addr = int(str(info['addr']),0)
    cycs = int(str(info['cycs']),0)


    def code0():
        align(2)
        label('altVideoTable')
        space(16)

    def code1():
        label('.wait')
        STW(R22)
        label('.waitl')
        LD('videoY');XORW(R22);BNE('.waitl')
        RET()
        label('.stars')
        LD(R16);ST(R17+1)
        LD('entropy');ST(R17);SUBI(160);BGE('.ret')
        LDW('entropy');BLT('.ret')
        LD(vACH);ANDI(0x3f);POKE(R17)
        label('.ret')
        RET()
        
    def code2():
        nohop()
        label('smoothScroll')
        PUSH()
        BRA('.skip')
        label('.loop')
        LDI(226);CALLI('.wait')
        label('.skip')
        LDI(224);CALLI('.wait')
        _LDI(addr);STW('sysFn')
        # shift videoTable[16..222]
        _LDI(0x100+18);STW('sysArgs2')
        SUBI(2);STW('sysArgs0');PEEK();STW(R16)
        _LDI(206);SYS(cycs);
        # videoTable[222]=altVideoTable[0]
        _LDI(0x100+222);STW(R17);LD('altVideoTable');POKE(R17)
        # shift altVideoTable
        LDI('altVideoTable');STW('sysArgs0')
        ADDI(2);STW('sysArgs2')
        _LDI(14);SYS(cycs)
        # altVideoTable[14]=saved videoTable[16]
        LD(R16);ST(v('altVideoTable')+14)
        # zero new row
        _LDI('SYS_SetMemory_v2_54');STW('sysFn')
        _LDI(160);STW('sysArgs0')
        LDI(0);ST('sysArgs2');LD(R16);ST('sysArgs3')
        SYS(54)
        # add stars
        CALLI('.stars')
        # loop or return
        LDW(R8);SUBI(1);STW(R8);BNE('.loop')
        POP();RET()

    module(name='scroll.s',
           code=[('DATA', 'altVideoTable', code0, 16, 2),
                 ('PLACE', 'altVideoTable', 0x0, 0x100),
                 ('EXPORT', 'altVideoTable'),
                 ('EXPORT', 'smoothScroll'),
                 ('CODE', 'smoothScroll1', code1),
                 ('CODE', 'smoothScroll', code2) ] )

scope()

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
	
