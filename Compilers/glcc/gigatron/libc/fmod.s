
def scope():
    
    def code0():
        nohop()
        label('fmod')
        PUSH()
        _MOVF(F8, FAC)
        LDI(F11);_CALLI('_@_fmod')
        POP();RET()

    module(name='fmod.s',
           code=[ ('EXPORT', 'fmod'),
                  ('IMPORT', '_@_fmod'),
                  ('CODE', 'fmod', code0) ] )

    def code1():
        label('_fmodquo')
        PUSH()
        _MOVF(F8, FAC)
        LDI(F11);_CALLI('_@_fmod')
        DOKE(R14) # low bits of quotient
        POP();RET()
        
    module(name='fmodquo.s',
           code=[ ('EXPORT', '_fmodquo'),
                  ('IMPORT', '_@_fmod'),
                  ('CODE', '_fmodquo', code1) ] )

    
scope()

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
