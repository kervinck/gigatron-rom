def scope():

    def code0():
        nohop()
        label('ldexp')
        PUSH()
        _MOVF(F8, FAC)
        LDW(R11)
        _FSCALB()
        POP();RET()
        
    module(name='ldexp.s',
           code=[ ('EXPORT', 'ldexp'),
                  ('CODE', 'ldexp', code0) ] )

    def code0():
        nohop()
        label('frexp')
        PUSH()
        _MOVF(F8,FAC)
        _CALLJ('_@_frexp')
        DOKE(R11)
        POP();RET()

    module(name='frexp.s',
           code=[ ('EXPORT', 'frexp'),
                  ('IMPORT', '_@_frexp'),
                  ('CODE', 'frexp', code0) ] )
	
scope()

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
