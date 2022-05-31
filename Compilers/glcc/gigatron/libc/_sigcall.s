
# Call signal subroutine.
# This is only imported when signal() is used.
# Registers R8-R22 must be saved to make sure the signal can return.
# Callee-saved registers R0-R7 need not be saved because
# the signal routine saves them as needed.

def code0():
    '''Redirected from _@_raise with vLR saved in [SP].'''
    nohop()
    label('_raise_emits_signal')
    ALLOC(-2);STLW(0)
    # create a stack frame and save R8-R22
    _SP(-38);STW(SP);ADDI(8);STW(T2)
    if args.cpu >= 6:
        LDI(R8);NCOPY(30)
    else:
        LDI(R8);STW(T0);LDI(R23);STW(T1);_CALLJ('_@_wcopy_')
    # call _sigcall(signo,fpeinfo)
    LDLW(0);ST(R8);LD(vACH);STW(R9);ALLOC(2)
    LDW(T3);STW(R10);_CALLJ('_sigcall');STW(T3)
    # restore R8-R22 and SP
    LDI(R8);STW(T2)
    if args.cpu >= 6:
        _SP(8);NCOPY(30);STW(SP)
    else:
        _SP(8);STW(T0);ADDI(30);STW(T1);STW(SP);_CALLJ('_@_wcopy_')
    # return to vLR saved by raise()
    LDW(SP);DEEK();tryhop(5);STW(vLR);LDW(T3);RET()

def code1():
    '''vIRQ handler'''
    nohop()
    label('_virq_handler')
    # save vLR/T0-T3 without using registers
    # skip 2 stack bytes because a lot of code uses STLW(-2)/LDLW(2) to save AC
    ALLOC(-10);LDW(T0);STLW(0);LDW(T1);STLW(2);LDW(T2);STLW(4);LDW(T3);STLW(6);PUSH()
    # clear virq vector
    LDWI('vIRQ_v5');STW(T0);LDI(0);DOKE(T0)
    # save sysFn/sysArgs[0-7]/B[0-2]/LAC
    LDW(SP);SUBI(22);STW(SP);ADDI(2);STW(T2)
    if args.cpu >= 6:
        LDI('_runbase');NCOPY(8);LDI('sysFn');NCOPY(10)
    else:
        LDI('_runbase');STW(T0);ADDI(8);STW(T1);_CALLJ('_@_wcopy_')
        LDI('sysFn');STW(T0);LDI(v('sysArgs7')+1);STW(T1);_CALLJ('_@_wcopy_')
    LDWI('.rti');DOKE(SP)
    LDI(0);STW(T3);LDI(7);_CALLI('_raise_emits_signal')

def code2():
    '''vIRQ return'''
    nohop()
    label('.rti')    # restore...
    if args.cpu >= 6:
        LDI(2);ADDW(SP);MOVQW('_runbase',T2);NCOPY(8);MOVQW('sysFn',T2);NCOPY(10);STW(SP)
    else:
        LDI(2);ADDW(SP);STW(T0);ADDI(8);STW(T1);LDI('_runbase');STW(T2);_CALLJ('_@_wcopy_')
        LDI(10);ADDW(T1);STW(T1);STW(SP);LDI('sysFn');STW(T2);_CALLJ('_@_wcopy_')
    POP();LDLW(0);STW(T0);LDLW(2);STW(T1);LDLW(4);STW(T2);LDLW(6);STW(T3);ALLOC(10)
    LDWI(0x400);LUP(0)

module(name='_sigcall.s',
       code=[ ('IMPORT', '_sigcall'),
              ('IMPORT', '_@_wcopy_') if args.cpu < 6 else ('NOP',),
              ('EXPORT', '_raise_emits_signal'),
              ('EXPORT', '_virq_handler'),
              ('CODE', '_raise_emits_signal', code0),
              ('CODE', '_virq_handler', code1),
              ('CODE', '.rti', code2) ] )

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
	
