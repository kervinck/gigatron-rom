
def scope():
    
    def code0():
        '''double _polevl(double x, double *coeff, char n)
           -- computes sum_{i=0}^{n} coeff[i]*x^{n-i}
           double _p1evl(double x, double *coeff, char n)
           -- same but coeff of x^n is 1.'''
        nohop()
        label('_p1evl')
        PUSH()
        _MOVF(F8,FAC);LDW(R11);_FADD()
        BRA('.polevl2')
        label('_polevl')
        PUSH()              # x is F8, *coeff is R11, n is R12
        LDW(R11);_MOVF([vAC],FAC)
        INC(R12);BRA('.polevl2')
        label('.polevl1')
        LDI(F8);_FMUL();LDW(R11);_FADD()
        label('.polevl2')
        if args.cpu >= 7:
            ADDSV(5,R11)
        else:
            LDW(R11);ADDI(5);STW(R11)
        if args.cpu >= 6:
            DBNE(R12,'.polevl1')
        else:
            LD(R12);SUBI(1);ST(R12);_BNE('.polevl1')
        tryhop(2);POP();RET()

    module(name='_polevl.s',
           code=[ ('EXPORT', '_polevl'),
                  ('EXPORT', '_p1evl'),
                  ('CODE', '_polevl', code0) ] )
    
scope()

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
