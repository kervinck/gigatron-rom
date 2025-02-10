
# ldiv_t ldiv(long a, long q)

def code0():
    label('ldiv')
    PUSH();
    _MOVL(L9,LAC);LDI(L11);_LMODS()
    if args.cpu >= 6:
        LDW(R8);DOKEA(T0);ADDI(2);DOKEA(T1)
        ADDI(2);STLAC()
    else:
        # save quotient
        LDW(T0);DOKE(R8);
        LDI(2);ADDW(R8);STW(T3);LDW(T1);DOKE(T3);
        # save remainder
        LDI(4);ADDW(R8);STW(T3);LDW(LAC);DOKE(T3);
        LDI(6);ADDW(R8);STW(T3);LDW(LAC+2);DOKE(T3);
    # return
    tryhop(2);POP();RET()
    
# ======== (epilog)
code=[
    ('EXPORT', 'ldiv'),
    ('CODE', 'ldiv', code0),
    ('IMPORT', '_@_lmods') ]

module(code=code, name='ldiv.s');

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
	
