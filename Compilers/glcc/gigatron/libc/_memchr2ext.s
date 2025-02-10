
def scope():

    code = []

    if 'has_SYS_ScanMemoryExt' in rominfo:
        info = rominfo['has_SYS_ScanMemoryExt']
        addr = int(str(info['addr']),0)
        cycs = int(str(info['cycs']),0)
        def m_prepScanMemoryExt():
            _MOVIW(addr,'sysFn')
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


    # void *__memchr2ext(char bank, const void *s, int c0c1, size_t len)
    # - scans at most n bytes from s until finding one equal to c0 or c1
    # - return pointer to the byte if found, 0 if not found.

    def code1():
        label('__memchr2ext');   # R8=bank R9=d, R10=c0c1, R11=len
        PUSH();
        LD(R10);STW('sysArgs2')
        LDW(R9);STW('sysArgs0');ADDW(R11);STW(R12)
        m_prepScanMemoryExt()
        label('.loop')
        LDW(R12);XORW('sysArgs0');_BEQ('.done')
        LD(vACH);_BNE('.s1')
        LDW(R12);SUBW('sysArgs0');_BLT('.s1')
        m_ScanMemoryExt()
        tryhop(2);POP();RET()
        label('.s1')
        LDI(0);SUBW('sysArgs0')
        m_ScanMemoryExt()
        INC('sysArgs1');_BEQ('.loop')
        label('.done')
        tryhop(2);POP();RET()

    code.append(('EXPORT', '__memchr2ext'))
    code.append(('CODE', '__memchr2ext', code1))

    module(name='memchr2ext.s', code=code)

    # void *_memchr2ext(char bank, const void *s, char c0, char c1, size_t len)
    # - scans at most n bytes from s until finding one equal to c0 or c1
    # - return pointer to the byte if found, 0 if not found.
    def code2():
        nohop(9)
        label('_memchr2ext');   # R8=bank R9=d, R10=c0, R11=c1, R12=len
        LD(R11);ST(R10+1);LDW(R12);STW(R11)
        if args.cpu >= 6:
            JNE('__memchr2ext')
        else:
            PUSH();_CALLJ('__memchr2ext');POP()
        RET()

    module(name='_memchr2ext.s',
           code=[('EXPORT', '_memchr2ext'),
                 ('IMPORT', '__memchr2ext'),
                 ('CODE', '_memchr2ext', code2) ] )

scope()

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
