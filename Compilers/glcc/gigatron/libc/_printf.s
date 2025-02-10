def scope():

    def code_printf():
        nohop()
        label('printf')
        _SP(2);STW(R9)
        if args.cpu >= 7:
            JNE('vprintf')
        else:
            PUSH();_CALLJ('vprintf')
            tryhop(2);POP();RET()

    module(name='printf.s',
           code=[('EXPORT', 'printf'),
                 ('IMPORT', 'vprintf'),
                 ('CODE', 'printf', code_printf) ] )


    def code_cprintf():
        nohop()
        label('cprintf')
        _SP(2);STW(R9)
        if args.cpu >= 7:
            JNE('vcprintf')
        else:
            PUSH();_CALLJ('vcprintf')
            tryhop(2);POP();RET()

    module(name='cprintf.s',
           code=[('EXPORT', 'cprintf'),
                 ('IMPORT', 'vcprintf'),
                 ('CODE', 'cprintf', code_cprintf) ] )


    def code_sprintf():
        nohop()
        label('sprintf')
        _SP(4);STW(R10)
        if args.cpu >= 7:
            JNE('vsprintf')
        else:
            PUSH();_CALLJ('vsprintf')
            tryhop(2);POP();RET()

    module(name='sprintf.s',
           code=[('EXPORT', 'sprintf'),
                 ('IMPORT', 'vsprintf'),
                 ('CODE', 'sprintf', code_sprintf) ] )

    def code_snprintf():
        nohop()
        label('snprintf')
        _SP(6);STW(R11)
        if args.cpu >= 7:
            JNE('vsnprintf')
        else:
            PUSH();_CALLJ('vsnprintf')
            tryhop(2);POP();RET()

    module(name='snprintf.s',
           code=[('EXPORT', 'snprintf'),
                 ('IMPORT', 'vnsprintf'),
                 ('CODE', 'snprintf', code_snprintf) ] )

    def code_fprintf():
        nohop()
        label('fprintf')
        _SP(4);STW(R10)
        if args.cpu >= 7:
            JNE('vfprintf')
        else:
            PUSH();_CALLJ('vfprintf')
            tryhop(2);POP();RET()

    module(name='fprintf.s',
           code=[('EXPORT', 'fprintf'),
                 ('IMPORT', 'vfprintf'),
                 ('CODE', 'fprintf', code_fprintf) ] )

scope()

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
	
