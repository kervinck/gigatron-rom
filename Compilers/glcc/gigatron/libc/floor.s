
def scope():
    
    def code0():
        nohop()
        label('modf')
        PUSH()
        _MOVF(F8, FAC)
        _CALLJ('_@_frndz')
        LDW(R11)
        _MOVF(FAC, [vAC])
        _FNEG();LDI(F8);_FADD()
        POP();RET()

    module(name='modf.s',
           code=[ ('EXPORT', 'modf'),
                  ('IMPORT', '_@_frndz'),
                  ('CODE', 'modf', code0) ] )

    SIGN = 0x81   # sign byte
    EXP = 0x82    # exponent
    
    def code0():
        nohop()
        label('floor')
        PUSH()
        _MOVF(F8, FAC)
        _CALLJ('_@_frndz')
        LDI(F8);_FCMP();_BLE('.ret')
        LDWI('_fone');_FSUB()
        label('.ret')
        POP();RET()
    
    module(name='floor.s',
           code=[ ('EXPORT', 'floor'),
                  ('IMPORT', '_@_frndz'),
                  ('IMPORT', '_fone'),
                  ('CODE', 'floor', code0) ] )

    def code0():
        nohop()
        label('ceil')
        PUSH()
        _MOVF(F8, FAC)
        _CALLJ('_@_frndz')
        LDI(F8);_FCMP();_BGE('.ret')
        LDWI('_fone');_FADD()
        label('.ret')
        POP();RET()
    
    module(name='ceil.s',
           code=[ ('EXPORT', 'ceil'),
                  ('IMPORT', '_@_frndz'),
                  ('IMPORT', '_fone'),
                  ('CODE', 'ceil', code0) ] )

scope()

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
