
def scope():

    # -- int _doprint(const char*, __va_list);
    # aliased to either _doprint_c89 or _doprint_simple

    doprint_default = '_doprint_c89'
    if 'PRINTF_C89' in args.opts:
        doprint_default = '_doprint_c89'
    elif 'PRINTF_SIMPLE' in args.opts:
        doprint_default = '_doprint_simple'

    def code_doprint():
        label('_doprint', doprint_default)

    module(name='doprint.s',
           code=[('EXPORT','_doprint'),
                 ('IMPORT', doprint_default),
                 ('CODE','_doprint',code_doprint) ] )


    # - struct _doprint_dst_s _doprint_dst
    def code_doprint_dst():
        align(2)
        label('_doprint_dst')
        words(0,0,0)

    module(name='doprint_dst.s',
           code=[('EXPORT','_doprint_dst'),
                 ('DATA','_doprint_dst',code_doprint_dst, 6, 2) ] )


    # - void _doprint_puts(const char *s, size_t len)
    def code_doprint_puts():
        nohop()
        label('_doprint_puts')
        PUSH();_ALLOC(-6)
        LDWI(v('_doprint_dst'));STW(R23);DEEK();ADDW(R9);DOKE(R23)
        LDWI(v('_doprint_dst')+2);DEEK();STW(R10) # append extra argument
        LDWI(v('_doprint_dst')+4);DEEK();CALL(vAC)
        _ALLOC(6);POP();RET()

    module(name='doprint_puts.s',
           code=[('EXPORT', '_doprint_puts'),
                 ('IMPORT', '_doprint_dst'),
                 ('CODE', '_doprint_puts', code_doprint_puts) ] )


    # - void _doprint_putc(int c, size_t cnt)
    def code_doprint_putc():
        label('_doprint_putc')
        _PROLOGUE(12,6,0xc0)
        _SP(12);STW(R6);LD(R8);DOKE(R6)
        LDWI(v('_doprint_dst'));STW(R23);DEEK();ADDW(R9);DOKE(R23)
        LDW(R9)
        _BRA('.tst')
        label('.loop')
        SUBI(1);STW(R7)
        _MOVW(R6,R8)
        _MOVIW(1,R9)
        LDWI(v('_doprint_dst')+2);DEEK();STW(R10)
        LDWI(v('_doprint_dst')+4);DEEK();CALL(vAC)
        LDW(R7)
        label('.tst')
        _BNE('.loop')
        _EPILOGUE(12,6,0xc0,saveAC=False)

    module(name='doprint_putc.s',
           code=[('EXPORT', '_doprint_putc'),
                 ('IMPORT', '_doprint_dst'),
                 ('CODE', '_doprint_putc', code_doprint_putc) ] )

scope()


# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
