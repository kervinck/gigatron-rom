
def code0():
    nohop()
    label('abs')
    LDW(R8)
    _BGE('.ret')
    if args.cpu >= 6:
        NEGV(vAC)
    else:
        LDI(0);SUBW(R8)
    label('.ret')
    RET()
    
code=[
    ('EXPORT', 'abs'),
    ('CODE', 'abs', code0) ]

module(code=code, name='abs.s');

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
	
