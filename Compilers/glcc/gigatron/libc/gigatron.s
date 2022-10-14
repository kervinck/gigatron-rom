
def scope():

    # ----------------------------------------
    # int SYS_Lup(unsigned int addr)
    #   Notes: This is not a real SYS call. Just the LUP instruction.
    def code0():
        nohop()
        label('SYS_Lup')
        LDW(R8);LUP(0);RET()

    module(name='sys_lup.s',
           code=[('EXPORT', 'SYS_Lup'),
                 ('CODE', 'SYS_Lup', code0) ])

    # ----------------------------------------
    # unsigned int SYS_Random(void);
    def code0():
        nohop()
        label('SYS_Random')
        _LDI('SYS_Random_34');STW('sysFn')
        SYS(34);RET()

    module(name='sys_random.s',
           code=[('EXPORT', 'SYS_Random'),
                 ('CODE', 'SYS_Random', code0) ])

    # ----------------------------------------
    # void SYS_VDrawBits(int fgbg, char bits, char *addr);
    def code0():
        nohop()
        label('SYS_VDrawBits')
        _LDI('SYS_VDrawBits_134');STW('sysFn')
        LDW(R8);STW('sysArgs0')
        LD(R9);ST('sysArgs2')
        LDW(R10);STW('sysArgs4')
        SYS(134);RET()

    module(name='sys_vdrawbits.s',
           code=[('EXPORT', 'SYS_VDrawBits'),
                 ('CODE', 'SYS_VDrawBits', code0) ])

    # ----------------------------------------
    # void SYS_Exec(void *romptr, void *vlr)
    def code0():
        nohop()
        label('SYS_Exec')
        _LDI('SYS_Exec_88');STW('sysFn')
        LDW(R8);STW('sysArgs0')
        LDW(R9);BEQ('.se1');STW(vLR)
        label('.se1')
        SYS(88);RET()

    module(name='sys_exec.s',
           code=[('EXPORT', 'SYS_Exec'),
                 ('CODE', 'SYS_Exec', code0) ])


    # ----------------------------------------
    # void SYS_SetMode(int)
    def code0():
        nohop();
        label('SYS_SetMode')
        _LDI('SYS_SetMode_v2_80');STW('sysFn')
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
        _LDI('SYS_ReadRomDir_v5_80');STW('sysFn')
        LDW(R8);SYS(80);STW(R8)
        LDW(R9);_MOVM('sysArgs0',[vAC],8,2)
        POP();LDW(R8);RET()

    module(name='sys_readromdir.s',
           code=[('EXPORT', 'SYS_ReadRomDir'),
                 ('CODE', 'SYS_ReadRomDir', code0),
                 ('IMPORT', '_@_bcopy_') if args.cpu < 6 else ('NOP',) ])


    # ----------------------------------------
    # void SYS_ExpanderControl(unsigned int ctrl);
    def code0():
        nohop()
        label('SYS_ExpanderControl')
        _LDI('SYS_ExpanderControl_v4_40');STW('sysFn')
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
        _LDI('SYS_SpiExchangeBytes_v4_134');STW('sysFn')
        # sysArgs[0]      Page index start, for both send/receive (in, changed)
        # sysArgs[1]      Memory page for send data (in)
        # sysArgs[2]      Page index stop (in)
        # sysArgs[3]      Memory page for receive data (in)
        LDW(R9);STW('sysArgs0')
        LD(R10);ST('sysArgs2')
        LD(R8+1);ST('sysArgs3')
        SYS(134)
        RET()
    
    module(name='sys_spiexchangebytes.s',
           code=[('EXPORT', 'SYS_SpiExchangeBytes'),
                 ('CODE', 'SYS_SpiExchangeBytes', code0) ])


    # int SYS_OsCall(unsigned char n);
    #    Notes: This only exists in the exp ROM. */
    if 'has_SYS_OsCall' in rominfo:
        info = rominfo['has_SYS_OsCall']
        addr = int(str(info['addr']),0)
        cycs = int(str(info['cycs']),0)
        def code0():
            nohop()
            label('SYS_OsCall')
            PUSH()
            _LDI(addr);STW('sysFn')
            LD(R8);SYS(cycs)
            tryhop(2);POP();RET()

        module(name='sys_oscall.s',
               code=[('EXPORT', 'SYS_OsCall'),
                     ('CODE', 'SYS_OsCall', code0) ])
    
# execute    
scope()

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
