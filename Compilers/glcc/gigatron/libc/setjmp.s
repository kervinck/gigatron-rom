

# int setjmp(jmp_buf)

def code0():
    label('setjmp')
    tryhop(4);LDW(vLR);STW(R22)
    # save SP, vLR, vSP, R0-R7
    if args.cpu >= 6:
        LDW(R8);DOKEA(SP);ADDI(2);DOKEA(R22)
        ADDI(2);DOKEA(vSP);ADDI(2);STW(T2)
        MOVQW(R0,T3);COPYN(16)
    else:
        LDW(SP);DOKE(R8);LDI(2);ADDW(R8);STW(R8)
        LDW(R22);DOKE(R8);LDI(2);ADDW(R8);STW(R8)
        LDW(vSP);DOKE(R8);LDI(2);ADDW(R8);STW(R8)
        LDI(R0);STW(T3);LDW(R8);STW(T2);LDI(R8);STW(T1);_CALLJ('_@_wcopy')
    # return 0
    LDW(R22);tryhop(5);STW(vLR);LDI(0);RET()

# void longjmp(jmp_buf, int)

def code1():
    label('longjmp')
    # restore SP, R22, R0-R7
    if args.cpu >= 6:
        LDW(R8);DEEKA(SP);ADDI(2);DEEKA(R22)
        if vSP != SP:
            ADDI(2);DEEK()
            STW(vSP) if args.cpu >= 7 else ST(vSP)
    else:
        LDW(R8);DEEK();STW(SP)
        LDI(2);ADDW(R8);DEEK();STW(R22);
        LDI(4);ADDW(R8);DEEK();ST(vSP);
    LDI(6);ADDW(R8);STW(T3)
    if args.cpu >= 6:
        MOVQW(R0,T2);COPYN(16)
    else:
        ADDI(16);STW(T1);LDI(R0);STW(T2);_CALLJ('_@_wcopy')
    # return R9
    LDW(R22);tryhop(5);STW(vLR);LDW(R9);RET()
    
module(name='setjmp.s',
       code=[ ('EXPORT', 'setjmp'),
              ('EXPORT', 'longjmp'),
              ('IMPORT', '_@_wcopy') if args.cpu < 6 else ('NOP',),
              ('CODE', 'setjmp', code0),
              ('CODE', 'longjmp', code1) ])

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
