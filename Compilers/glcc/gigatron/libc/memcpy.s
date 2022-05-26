
# This variant uses only the NCOPY opcode. Requires cpu6.
def scope_ncopy():
    def code0():
        nohop()
        label('memcpy');                            # R8=d, R9=s, R10=l
        PUSH()
        LDW(R8);STW(vDST)
        label('.memcpy1')
        LDW(R10);_BEQ('.memcpy3')
        LD(R10+1);_BEQ('.memcpy2')
        LDW(R9);NCOPY(0);STW(R9)
        DEC(R10+1);_BRA('.memcpy1')
        label('.memcpy2')
        LDWI(v('.ncopy')+1);POKEA(R10)
        LDW(R9);label('.ncopy');NCOPY(1)
        label('.memcpy3')
        LDW(R8);tryhop(2);POP();RET()

    module(name='memcpy.s',
           code=[('EXPORT', 'memcpy'),
                 ('CODE', 'memcpy', code0) ])
    

# This longer variant uses SYS_CopyMemory.
def scope_syscopymemory():
    info = rominfo['has_SYS_CopyMemory']
    addr = int(str(info['addr']),0)
    cycs = int(str(info['cycs']),0)
    def code0():
        label('memcpy');                            # R8=d, R9=s, R10=l
        PUSH()
        LDWI(addr);STW('sysFn')
        LDW(R8);STW(R21);STW('sysArgs0')
        LDW(R9);STW('sysArgs2')
        label('.loop')
        LD(R21);STW(R20)
        LD(R9);SUBW(R20);_BLE('.memcpy1')
        LD(R9);STW(R20)
        label('.memcpy1')
        LDI(255);ST(R20+1)                          # R20 is minus count to page boundary
        LDW(R10);_BGT('.memcpy2')
        _BEQ('.done')                               # a) len is zero
        ADDW(R20);_BRA('.memcpy4')                  # b) len is larger than 0x8000
        label('.memcpy2')
        ADDW(R20);_BLE('.memcpy5')                  # c) len is smaller than -R20
        label('.memcpy4')
        STW(R10)                                    # d) len is larger than -R20
        LDI(0);SUBW(R20);STW(R20);SYS(cycs)
        LDW(R21);ADDW(R20);STW(R21);STW('sysArgs0')
        LDW(R9);ADDW(R20);STW(R9);STW('sysArgs2')
        _BRA('.loop')
        label('.memcpy5')
        LDW(R10);SYS(cycs)
        label('.done')
        LDW(R8);tryhop(2);POP();RET();
           
    module(name='memcpy.s',
           code=[('EXPORT', 'memcpy'),
                 ('CODE', 'memcpy', code0) ])
           

# This variant uses vCPU only
def scope_vcpu():
           
    def code0():
        nohop()
        # copy without page crossings
        # takes destination ptr in sysArgs0/1
        # takes source ptr in sysArgs2/3
        # takes length in sysArgs4 (not vAC)
        label('_memcpy0')
        # single byte
        LD('sysArgs4');ANDI(1);BEQ('.cpy2')
        _PEEKV('sysArgs2');POKE('sysArgs0')
        INC('sysArgs2');INC('sysArgs0')
        LD('sysArgs4');ANDI(0xfe);ST('sysArgs4');BEQ('.cpydone')
        # even length
        label('.cpy2')
        if args.cpu <= 5:
           label('.cpy2loop')
           LDW('sysArgs2');DEEK();DOKE('sysArgs0')
           INC('sysArgs2');INC('sysArgs0')
           INC('sysArgs2');INC('sysArgs0')
           LD('sysArgs4');SUBI(2);ST('sysArgs4');BNE('.cpy2loop')
        else:
           label('.cpy2loop')
           DEEKp('sysArgs2'); DOKEp('sysArgs0')
           DEC('sysArgs4'); DBNE('sysArgs4', '.cpy2loop')
        label('.cpydone')
        RET()

    def code1():
        label('memcpy');                            # R8=d, R9=s, R10=l
        PUSH()
        LDW(R8);STW(R21);STW('sysArgs0')
        LDW(R9);STW('sysArgs2')
        label('.loop')
        LD(R8);STW(R20)
        LD(R9);SUBW(R20);_BLE('.memcpy1')
        LD(R9);STW(R20)
        label('.memcpy1')
        LDI(255);ST(R20+1)                          # R20 is minus count to page boundary
        LDW(R10);_BGT('.memcpy2')
        _BEQ('.done')                               # a) len is zero
        ADDW(R20);_BRA('.memcpy4')                  # b) len is larger than 0x8000
        label('.memcpy2')
        ADDW(R20);_BLE('.memcpy5')                  # c) len is smaller than -R20
        label('.memcpy4')
        STW(R10)                                    # d) len is larger than -R20
        LDI(0);SUBW(R20);STW(R20)
        STW('sysArgs4');_CALLJ('_memcpy0')
        LDW(R8);ADDW(R20);STW(R8);STW('sysArgs0')
        LDW(R9);ADDW(R20);STW(R9);STW('sysArgs2')
        _BRA('.loop')
        label('.memcpy5')
        LDW(R10);STW('sysArgs4');_CALLJ('_memcpy0')
        label('.done')
        LDW(R21);tryhop(2);POP();RET();

    module(name='memcpy.s',
           code=[('EXPORT', 'memcpy'),
                 ('CODE', '_memcpy0', code0),
                 ('CODE', 'memcpy', code1) ])


if args.cpu >= 6 and False:
    scope_ncopy()
elif 'has_SYS_CopyMemory' in rominfo:
    scope_syscopymemory()
else:
    scope_vcpu()

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
