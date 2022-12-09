
def scope():
    
    code = [ ('EXPORT', '_memchr2ext') ]

    if 'has_SYS_ScanMemoryExt' in rominfo:
        info = rominfo['has_SYS_ScanMemoryExt']
        addr = int(str(info['addr']),0)
        cycs = int(str(info['cycs']),0)
        def m_prepScanMemoryExt():
            LDWI(addr);STW('sysFn')
            LD(R8);ST(R17+1) # R17 = R8 << 8
        def m_ScanMemoryExt():
            # scan memory without page crossings
            # takes data ptr in sysArgs0/1
            # takes two byte targets in sysArgs2/3
            # takes length in vACL (0 means 256)
            # returns pointer to target or 0
            ST(R17);LDW(R17);SYS(cycs)
    else:
        def m_prepScanMemoryExt():
            LDWI(0x1f8);PEEK();STW(R16);_BNE('.ok')        # R16: copy of 1f8
            LDI(0);tryhop(2);POP();RET();label('.ok')
            LDW(R8);ORI(0x3c);ANDI(0xfc);STW(R17)          # R17: target ctrl word
            _LDI('SYS_ExpanderControl_v4_40');STW('sysFn')
        def m_ScanMemoryExt():
            STW('sysArgs4');_CALLJ('_memscan0ext')
        def code0():
            # scan memory without page crossings
            # takes data ptr in sysArgs0/1
            # takes two byte targets in sysArgs2/3
            # takes length in sysArgs4 (not vACL)
            # returns pointer to target or 0
            nohop()
            label('_memscan0ext')
            LDW(R17);SYS(40)
            LDI(0);SUBW('sysArgs4');STW('sysArgs4')
            label('.scanloop')
            LDW('sysArgs0');PEEK()
            ST(vACH);XORW('sysArgs2');STW(T3)
            LD(T3);_BEQ('.scanok')
            LD(T3+1);_BEQ('.scanok')
            INC('sysArgs0');INC('sysArgs4')
            LD('sysArgs4');_BNE('.scanloop')
            LDW(R16);SYS(40)
            LDI(0);RET()
            label('.scanok')
            LDW(R16);SYS(40)
            LDW('sysArgs0');RET()

        code.append(('CODE', '_memscan0ext', code0))
        code.append(('PLACE', '_memscan0ext', 0x0200, 0x7fff))


    # void *_memchr2ext(char bank, const void *s, char c0, char c1, size_t len)
    # - scans at most n bytes from s until finding one equal to c0 or c1
    # - return pointer to the byte if found, 0 if not found.

    def code1():
        label('_memchr2ext');    # R8=bank R9=d, R10=c0 R11=c1, R12=len
        PUSH();
        LDW(R9);STW('sysArgs0')
        LD(R10);ST('sysArgs2');LD(R11);ST('sysArgs3')
        m_prepScanMemoryExt()
        label('.loop')
        LD(R9);ST(R20);LDI(255);ST(R20+1)           # R20 is minus count to end of block
        LDW(R12);_BGE('.memscan2')
        ADDW(R20);_BRA('.memscan4')                 # b) len is larger than 0x8000
        label('.memscan2')
        _BEQ('.done')                               # a) len is zero
        ADDW(R20);_BLE('.memscan5')                 # c) len is smaller than -R20
        label('.memscan4')
        STW(R12)                                    # d) len is larger than -R20
        LDI(0);SUBW(R20);STW(R20)
        m_ScanMemoryExt();_BNE('.done')
        LDW(R9);ADDW(R20);STW(R9);STW('sysArgs0')
        _BRA('.loop')
        label('.memscan5')
        LDW(R12);m_ScanMemoryExt()
        label('.done')
        tryhop(2);POP();RET();

    code.append(('CODE', '_memchr2ext', code1))

    
    return code;

module(code=scope(), name='memchr.s');

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
