
def scope():

    # ----------------------------------------
    # Channel access function

    def code_channel():
        nohop()
        label('channel')
        LD(R8);ST(vACH);ORI(0xff);XORI(5);RET()  # nine bytes

        module(name='channel.s',
           code=[('EXPORT', 'channel'),
                 ('CODE', 'channel', code_channel) ] )

    # ----------------------------------------
    # int SYS_Lup(unsigned int addr)
    # -- Not a sys call but a stub for the LUP opcode.
    def code0():
        nohop()
        label('SYS_Lup')
        LDW(R8);LUP(0);RET()

    module(name='sys_lup.s',
           code=[('EXPORT', 'SYS_Lup'),
                 ('CODE', 'SYS_Lup', code0) ])

    # ----------------------------------------
    # int SYS_Fill(unsigned int yyxx, char vv, unsigned int hhww)
    # -- Not a sys call but a stub for the FILL opcode.

    def code0():
        nohop()
        label('SYS_Fill')
        if args.cpu >= 7:
            MOVW(R8,T2)
            LD(R9);STW(T3)
            LDW(R10)
            FILL();
        RET()

    module(name='sys_fill.s',
           code=[('EXPORT', 'SYS_Fill'),
                 ('CODE', 'SYS_Fill', code0) ])


    # ----------------------------------------
    # int SYS_Blit(unsigned dydx, unsigned sysx, unsigned hhww);
    # -- Not a sys call but a stub for the BLIT opcode.
    
    def code0():
        nohop()
        label('SYS_Blit')
        if args.cpu >= 7:
            MOVW(R8,T2)
            MOVW(R9,T3)
            LDW(R10)
            BLIT();
        RET()

    module(name='sys_blit.s',
           code=[('EXPORT', 'SYS_Blit'),
                 ('CODE', 'SYS_Blit', code0) ])

    
    # ----------------------------------------
    # unsigned int SYS_Random(void);
    def code0():
        nohop()
        label('SYS_Random')
        _MOVIW('SYS_Random_34','sysFn')
        SYS(34);RET()

    module(name='sys_random.s',
           code=[('EXPORT', 'SYS_Random'),
                 ('CODE', 'SYS_Random', code0) ])

    # ----------------------------------------
    # void SYS_VDrawBits(int fgbg, char bits, char *addr);
    def code0():
        nohop()
        label('SYS_VDrawBits')
        _MOVIW('SYS_VDrawBits_134','sysFn')
        _MOVW(R8,'sysArgs0')
        LD(R9);ST('sysArgs2')
        _MOVW(R10,'sysArgs4')
        SYS(134);RET()

    module(name='sys_vdrawbits.s',
           code=[('EXPORT', 'SYS_VDrawBits'),
                 ('CODE', 'SYS_VDrawBits', code0) ])

    # ----------------------------------------
    # void SYS_Exec(void *romptr, void *vlr)
    def code0():
        nohop()
        label('SYS_Exec')
        _MOVIW('SYS_Exec_88','sysFn')
        _MOVW(R8,'sysArgs0')
        _LDI(-1);XORW(R9);_BEQ('.se1')
        _MOVW(R9,vLR)
        label('.se1')
        SYS(88);RET()

    module(name='sys_exec.s',
           code=[('EXPORT', 'SYS_Exec'),
                 ('CODE', 'SYS_Exec', code0) ])

    # ----------------------------------------
    # void SYS_SetMemory(int count, int val, void *addr);
    def code0():
        nohop()
        label('SYS_SetMemory')
        _MOVIW('SYS_SetMemory_v2_54','sysFn')
        LD(R8);ST('sysArgs0')
        LD(R9);ST('sysArgs1')
        _MOVW(R10,'sysArgs2')
        SYS(54);RET()

    module(name='sys_setmemory.s',
           code=[('EXPORT', 'SYS_SetMemory'),
                 ('CODE', 'SYS_SetMemory', code0) ])

    # ----------------------------------------
    # void SYS_SetMode(int)
    def code0():
        nohop();
        label('SYS_SetMode')
        _MOVIW('SYS_SetMode_v2_80','sysFn')
        LDW(R8);SYS(80);RET()

    module(name='sys_setmode.s',
           code=[('EXPORT', 'SYS_SetMode'),
                 ('CODE', 'SYS_SetMode', code0) ])

    # ----------------------------------------
    # void* SYS_ReadRomDir(void *romptr, char *buf8);
    def code0():
        nohop()
        label('SYS_ReadRomDir')
        PUSH()
        _MOVIW('SYS_ReadRomDir_v5_80','sysFn')
        LDW(R8);SYS(80);STW(R8)
        _MOVW(R9,T2)
        _MOVIW('sysArgs0',T3)
        if args.cpu >= 6:
            MOVQB(8,R10)
            label('.loop')
            PEEKV(T3);POKE(T2)
            INCV(T2);INCV(T3)
            DBNE(R10,'.loop')
        else:
            label('.loop')
            _PEEKV(T3);POKE(T2)
            LDI(1);ADDW(T2);STW(T2)
            LDI(1);ADDW(T3);STW(T3)
            XORI(v('sysArgs0')+8)
            _BNE('.loop')
        POP();LDW(R8);RET()

    module(name='sys_readromdir.s',
           code=[('EXPORT', 'SYS_ReadRomDir'),
                 ('CODE', 'SYS_ReadRomDir', code0) ])


    # ----------------------------------------
    # void SYS_ExpanderControl(unsigned int ctrl);
    def code0():
        nohop()
        label('SYS_ExpanderControl')
        _MOVIW('SYS_ExpanderControl_v4_40','sysFn')
        LDW(R8);SYS(40)
        label('.ret')
        RET()

    module(name='sys_expandercontrol.s',
           code=[('EXPORT', 'SYS_ExpanderControl'),
                 ('CODE', 'SYS_ExpanderControl', code0) ])


    # ----------------------------------------
    # void SYS_SpiExchangeBytes(void *dst, void *src, void *srcend);
    #    Notes: This exists in v4 but depends on 0x81 containing ctrlBits.
    #    Notes: only the high 8 bits of `dst` are used.
    #    Notes: only the low 8 bits of `srcend` are used.
    def code0():
        nohop()
        label('SYS_SpiExchangeBytes')
        _MOVIW('SYS_SpiExchangeBytes_v4_134','sysFn')
        # sysArgs[0]      Page index start, for both send/receive (in, changed)
        # sysArgs[1]      Memory page for send data (in)
        # sysArgs[2]      Page index stop (in)
        # sysArgs[3]      Memory page for receive data (in)
        _MOVW(R9,'sysArgs0')
        LD(R10);ST('sysArgs2')
        LD(R8+1);ST('sysArgs3')
        SYS(134)
        RET()
    
    module(name='sys_spiexchangebytes.s',
           code=[('EXPORT', 'SYS_SpiExchangeBytes'),
                 ('CODE', 'SYS_SpiExchangeBytes', code0) ])

    # ----------------------------------------
    # void* SYS_Sprite6(void *srcpix, void *dst);
    def code():
        nohop()
        label('SYS_Sprite6')
        _MOVIW('SYS_Sprite6_v3_64','sysFn')
        _MOVW(R8,'sysArgs0')
        LDW(R9);SYS(64)
        RET()

    module(name='sys_sprite6.s',
           code=[('EXPORT', 'SYS_Sprite6'),
                 ('CODE', 'SYS_Sprite6', code) ] )
    
    # ----------------------------------------
    # void* SYS_Sprite6x(void *srcpix, void *dst);
    def code():
        nohop()
        label('SYS_Sprite6x')
        _MOVIW('SYS_Sprite6x_v3_64','sysFn')
        _MOVW(R8,'sysArgs0')
        LDW(R9);SYS(64)
        RET()

    module(name='sys_sprite6x.s',
           code=[('EXPORT', 'SYS_Sprite6x'),
                 ('CODE', 'SYS_Sprite6x', code) ] )

    # ----------------------------------------
    # void* SYS_Sprite6y(void *srcpix, void *dst);
    def code():
        nohop()
        label('SYS_Sprite6y')
        _MOVIW('SYS_Sprite6y_v3_64','sysFn')
        _MOV(R8,'sysArgs0')
        LDW(R9);SYS(64)
        RET()

    module(name='sys_sprite6y.s',
           code=[('EXPORT', 'SYS_Sprite6y'),
                 ('CODE', 'SYS_Sprite6y', code) ] )

    # ----------------------------------------
    # void* SYS_Sprite6xy(void *srcpix, void *dst);
    def code():
        nohop()
        label('SYS_Sprite6xy')
        _MOVIW('SYS_Sprite6xy_v3_64','sysFn')
        _MOVW(R8,'sysArgs0')
        LDW(R9);SYS(64)
        RET()

    module(name='sys_sprite6xy.s',
           code=[('EXPORT', 'SYS_Sprite6xy'),
                 ('CODE', 'SYS_Sprite6xy', code) ] )

# execute    
scope()

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
