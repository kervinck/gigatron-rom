
def scope():

    # -- void *memchr(const void *s, int c0, size_t n)
    # -- void *__memchr2(const void *s, int c0c1, size_t n)
    # scan at most n bytes from s until finding one equal to c0 or c1
    # return pointer to the byte if found, 0 if not found.
    # known to leave R8/R9/R10 unchanged!!!
    def code1():
        nohop()
        label('memchr')
        LD(R9);ST(R9+1)
        label('__memchr2')                          # R8=d, R9=c0c1, R10=l
        LDW(R9);STW('sysArgs2')
        LDW(R8);STW('sysArgs0');ADDW(R10);STW(R12)
        if 'has_SYS_ScanMemory' in rominfo:
            info = rominfo['has_SYS_ScanMemory']
            addr = int(str(info['addr']),0)
            cycs = int(str(info['cycs']),0)
            _MOVIW(addr,'sysFn')
            label('.loop')
            LDW(R12);XORW('sysArgs0');_BEQ('.done')
            LD(vACH);_BNE('.s1')
            LDW(R12);SUBW('sysArgs0');_BLT('.s1')
            SYS(cycs);RET()
            label('.s1')
            LDI(0);SUBW('sysArgs0')
            SYS(cycs);INC('sysArgs1');_BEQ('.loop')
            label('.done')
            RET()
        else:
            LDW('sysArgs0')
            label('.loop')
            XORW(R12);_BEQ('.done')
            LDW('sysArgs0');PEEK();ST(vACH);XORW('sysArgs2');ST(R13)
            LD(vACH);_BEQ('.ok')
            LD(R13);_BEQ('.ok')
            LDI(1);ADDW('sysArgs0');STW('sysArgs0')
            _BRA('.loop')
            label('.ok')
            LDW('sysArgs0')
            label('.done')
            RET()

    module(name='memchr.s',
           code=[('EXPORT', 'memchr'),
                 ('EXPORT', '__memchr2'),
                 ('CODE', 'memchr', code1) ] )


    # -- void *_memchr2(const void *s, char c0, char c1, size_t n)
    def code2():
        nohop()
        label('_memchr2');
        LD(R10);ST(R9+1);_MOVW(R11,R10)
        if args.cpu >= 6:
            JNE('__memchr2')
        else:
            PUSH();_CALLJ('__memchr2');POP()
        RET()

    module(name='_memchr2.s',
           code=[('EXPORT', '_memchr2'),
                 ('IMPORT', '__memchr2'),
                 ('CODE', '_memchr2', code2) ] )

    
    # -- int strlen(const char *s)
    def code3():
        nohop()
        label('strlen')
        PUSH()
        LDI(0);STW(R9)
        SUBI(1);STW(R10)
        _CALLJ('__memchr2')  # preserve R8!
        SUBW(R8)
        tryhop(2);POP();RET();

    module(name='strlen.s',
           code=[('EXPORT', 'strlen'),
                 ('IMPORT', '__memchr2'),
                 ('CODE', 'strlen', code3) ] )

scope()


# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
