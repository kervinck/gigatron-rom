
# Call signal subroutine.
# This is only imported when signal() is used.
# Registers R8-R22 must be saved to make sure the signal can return.
# Callee-saved registers R0-R7 need not be saved because
# the signal routine saves them as needed.

def code0():
    '''Redirected from _@_raise with vLR saved in [SP].
       Args: msg in T0, signo in T1, vSP%4 unknown.'''
    nohop()
    label('_raise_emits_signal')
    # align stack so that vSP%4==2, save SP, create stack frame
    LDW(SP);STW(T3);ORI(3);SUBI(5)
    if args.cpu < 6:
        STW(T4)
    else:
        DOKEA(T3)
    SUBI(38);STW(SP);ADDI(6);STW(T2)
    if args.cpu < 6:
        LDW(T3);DOKE(T4)
    LDW(T1);DOKE(SP)
    # create a stack frame and save R8-R23
    if args.cpu >= 6:
        _MOVIW(R8,T3);COPYN(32)
        # call _sigcall(signo,fpeinfo)
        LDW(SP);DEEK();ST(R8);LD(vACH);ST(R9)
        LDW(T0);STW(R10);CALLI('_sigcall');STW(T0)
        # restore R8-R22 and SP
        LDI(R8);STW(T2)
        _SP(6);STW(T3);ADDI(32);STW(SP)
        COPYN(32)
    else:
        LDI(R8);STW(T3);LDI(R8+32);STW(T1);_CALLJ('_@_wcopy')
        # call _sigcall(signo,fpeinfo)
        LDW(SP);DEEK();ST(R8);LD(vACH);ST(R9)
        LDW(T0);STW(R10);_CALLJ('_sigcall');STW(T0)
        # restore R8-R22 and SP
        LDI(R8);STW(T2)
        _SP(6);STW(T3);ADDI(32);STW(T1);STW(SP)
        _CALLJ('_@_wcopy')
    # restore SP
    _DEEKV(SP);STW(SP)
    # return to vLR saved by raise()
    tryhop(4);LDW(T0);POP();RET()

module(name='_sigcall.s',
       code=[ ('IMPORT', '_sigcall'),
              ('IMPORT', '_@_wcopy') if args.cpu < 6 else ('NOP',),
              ('EXPORT', '_raise_emits_signal'),
              ('CODE', '_raise_emits_signal', code0) ] )


# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
	
