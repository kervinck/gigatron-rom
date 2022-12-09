
def scope():
    
    def code0():
        '''void _memswp(char *R8, char *R9, size_t R10)'''
        nohop()
        label('_memswp')
        LDW(R8);STW(T2)
        LDW(R9);STW(T3)
        LDW(R10);_BEQ('.ret')
        label('.loop')
        if args.cpu >= 7:
            SUBI(4);_BLT('.loop1');STW(R10)
            LD(T2);SUBI(0xfc);_BGT('.loop1')
            LD(T3);SUBI(0xfc);_BGT('.loop1')
            LDW(T2);LDLAC();LDW(T3);COPYN(4);STLAC()
            LDW(R10);_BNE('.loop')
            RET()
        else:
            SUBI(2);_BLT('.loop1');STW(R10)
            LD(T2);SUBI(0xfe);_BGT('.loop1')
            LD(T3);SUBI(0xfe);_BGT('.loop1')
            _DEEKV(T2);STW(R11);_DEEKV(T3);DOKE(T2);LDW(R11);DOKE(T3)
            LDI(2);ADDW(R9);STW(R9)
            LDI(2);ADDW(R8);STW(R8)
            LDW(R10);_BNE('.loop')
            RET()
        label('.loop1')
        _PEEKV(T2);ST(R11);_PEEKV(T3);POKE(T2);LD(R11);POKE(T3)
        if args.cpu >= 6:
            INCV(T2);INCV(T3)
        else:
            LDI(1);ADDW(T3);STW(T3)
            LDI(1);ADDW(T2);STW(T2)
        LDW(R10);SUBI(1);STW(R10)
        _BNE('.loop')
        label('.ret')
        RET()

    module(name='_memswp.s',
           code=[('EXPORT', '_memswp'),
                 ('CODE', '_memswp', code0) ] )

scope()

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
