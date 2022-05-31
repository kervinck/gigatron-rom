
# double fabs(double);

def code0():
    nohop()
    label('fabs')
    PUSH()
    LD(F8+1);ANDI(127);ST(F8+1)
    _MOVF(F8, FAC)
    POP();RET()
    
code=[
    ('EXPORT', 'fabs'),
    ('CODE', 'fabs', code0) ]
	
module(code=code, name='fabs.s');

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
