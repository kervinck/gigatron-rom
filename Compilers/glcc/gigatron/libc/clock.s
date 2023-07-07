def scope():

    def code0():
        '''zero page variables'''
        label('_vIrqCounter')
        words(0)

    def code1():
        nohop()
        label('_vIrqHandler')
        LDI(1);ADDW('_vIrqCounter');STW('_vIrqCounter')
        label('_vIrqRelay', pc()+1)
        LDWI(0);_BEQ('.h1')
        PUSH();CALL(vAC);POP()
        label('.h1')
        LDWI(0x400);LUP(0)

    def code2():
        label('_vIrqInit')
        LDI(0);STW('_vIrqCounter');ST('frameCount')
        LDWI('_vIrqHandler');_BRA('.v1')
        label('_vIrqFini')
        LDI(0)
        label('.v1')
        STW(T3);LD('romType');ANDI(0xfc);SUBI(0x40);_BLT('.v2')
        LDWI('vIRQ_v5');STW(T2);LDW(T3);DOKE(T2)
        label('.v2')
        RET()

    def code3():
        label('__glink_magic_init')
        words('_vIrqInit', 0)

    def code4():
        label('__glink_magic_fini')
        words('_vIrqFini', 0)
        
    module(name='_virq.s',
           code=[ ('EXPORT', '_vIrqCounter'),
                  ('EXPORT', '_vIrqRelay'),
                  ('BSS', '_vIrqCounter', code0, 2, 1),
                  ('PLACE', '_vIrqCounter', 0x0000, 0x007f) ]
           +    ( [ ('CODE', '_vIrqHandler', code1),
                    ('CODE', '_vIrqInit', code2), 
                    ('PLACE', '_vIrqHandler', 0x200, 0x7fff),
                    ('DATA', '__glink_magic_init', code3, 4, 2),
                    ('DATA', '__glink_magic_fini', code4, 4, 2) ]
                  if 'has_vIRQ' in rominfo else [] ) )
    

    def code5():
        label('clock')
        label('_clock')
        if 'has_vIRQ' in rominfo:
            LD('frameCount');ST(LAC)
            LDW('_vIrqCounter');STW(LAC+1)
            LDI(0);ST(LAC+3)
            LDW(LAC);RET()
        else:
            warning('clock() cannot work without vIRQ (needs rom>=v5a)', dedup=True)
            LDI(0);STW(LAC);STW(LAC+2);RET()

    module(name='clock.s',
           code=[ ('EXPORT', 'clock'),
                  ('EXPORT', '_clock'),
                  ('IMPORT', '_vIrqCounter'),
                  ('CODE', 'clock', code5) ] )

    def code6():
        nohop()
        # Following at67: 179 is normally the start of vBlank, but if
        # a vBlank routine is executing there is a very good chance by
        # the time the vBlank routine is over giga_videoY will have
        # progressed past 179, (by how much is nondeterministic). So
        # instead we wait for the scanline before vBlank, i.e. when
        # videoY = 0xEE, (videoTablePtr = 0x01EE)
        label('.w1')
        LD('videoY');XORI(0xee);_BEQ('.w1')
        label('_wait')
        LD('videoY');XORI(0xee);_BNE('_wait')
        LDW(R8);SUBI(1);STW(R8);BGT('.w1')
        RET();

    module(name='_wait.s',
           code=[ ('EXPORT', '_wait'),
                  ('CODE', '_wait', code6) ] )
    
scope()

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
	
