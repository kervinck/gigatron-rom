
def scope():

    def code0():
        nohop()
        label('raise')                       # void raise(int signo);
        LDI(0);STW(R9)
        label('_raisem')                     # void _raisem(int signo, const char *msg);
        LDW(R8);ANDI(0xf8);BEQ('.raise1');
        _LDI(-1);RET()
        label('.raise1')
        LDW(R9);STW(T3)
        LDW(R8)
        label('__@raisem')                   # signo in vAC, msg in T3
        STLW(-2);
        label('_raise_disposition', pc()+1)
        LDWI(0)
        BEQ('.raise2')
        STW(T2);
        LDW(vLR);DOKE(SP);LDLW(-2);CALL(T2)  # dispatcher (no return)
        label('.raise2')
        LDI(20);STW(R8);
        LDW(T3);STW(R9);
        _CALLJ('_exitm')
        HALT()

    module(name='raise.s',
           code=[ ('IMPORT', '_exitm'),
                  ('EXPORT', 'raise'),
                  ('EXPORT', '_raisem'),
                  ('EXPORT', '__@raisem'),
                  ('EXPORT', '_raise_disposition'),
                  ('CODE', 'raise', code0) ] )

    def code1():
        nohop()
        label('_raise_sets_code')
        LDWI('_raise_code');STW(T2)
        LDLW(-2);DOKE(T2)
        LDW(SP);DEEK();STW(vLR);RET()
        align(2);
        label('_raise_code')
        words(0)

    module(name='raise_sets_code.s',
           code=[ ('EXPORT', '_raise_sets_code'),
                  ('EXPORT', '_raise_code'),
                  ('CODE', '_raise_sets_code', code1) ] )

scope()

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
	
