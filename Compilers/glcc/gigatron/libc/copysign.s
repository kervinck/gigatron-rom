
# double copysign(double, double);

def code0():
    nohop()
    label('copysign')
    PUSH()
    LD(F8+1);XORW(F11+1);ANDI(0x80);XORW(F8+1);ST(F8+1)
    _MOVF(F8,FAC)
    POP();RET()
    
code=[
    ('EXPORT', 'copysign'),
    ('CODE', 'copysign', code0) ]
	
module(code=code, name='copysign.s');

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
