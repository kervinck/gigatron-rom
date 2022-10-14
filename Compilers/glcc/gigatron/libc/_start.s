
### The rom/ram checking code must work on all cpu

def code0():
    ### _start()
    label('_start');
    # save vLR, vSP
    PUSH();LDWI('_exitvsp');STW(T3);LD(vSP);POKE(T3)
    # create stack headroom for argc and argv
    if args.cpu >= 6:
        SUBVI(4,SP)
    else:
        LDWI(-4);ADDW(SP);STW(SP)
    # initialize bss
    if not args.no_runtime_bss_initialization:
        _CALLJ('_init_bss')
    # call init chain
    LDWI('__glink_magic_init'); _CALLI('_callchain')
    # call main
    LDI(0); STW(R8); STW(R9); _CALLI('main'); STW(R8)
    ### exit()
    label('exit')
    LDW(R8); STW(R0)
    # call fini chain
    LDWI('__glink_magic_fini'); _CALLI('_callchain')
    LDW(R0); STW(R8)
    ### _exit()
    label('_exit')
    LDI(0); STW(R9)
    label('_exitm');
    LDW(R8);STW(R0)
    label('_exitvsp', pc()+1)
    LDI(0);ST(vSP)         # .exitvsp is LDI's argument!
    label('_exitm_msgfunc', pc()+1)
    LDWI(0);BEQ('.halt')   # _exitm_msgfunc is LDWI's argument here
    CALL(vAC)              # arguments in R8 and R9 are already correct
    # If _exitm_msgfunc is zero or returns
    # we just Flash a pixel with a position indicative of the return code
    label('.halt')
    LDWI(0x101);PEEK();ADDW(R0);ST(R7);
    LDWI(0x100);PEEK();ST(R7+1)
    label('.loop')
    POKE(R7)
    ADDI(1)
    BRA('.loop')

def code1():
    # subroutine to call a chain of init/fini functions
    nohop()
    label('_callchain')
    DEEK(); STW(R7); LDW(vLR); STW(R6)
    LDWI(0xBEEF);XORW(R7);_BEQ('.callchaindone')
    LDW(R7);_BRA('.callchaintst')
    label('.callchainloop')
    DEEK();CALL(vAC)
    LDI(2);ADDW(R7);DEEK();STW(R7)
    label('.callchaintst')
    _BNE('.callchainloop')
    label('.callchaindone')
    LDW(R6); STW(vLR); RET()

def code2():
    align(2)
    label('__glink_magic_init')
    words(0xBEEF)

def code3():
    align(2)
    label('__glink_magic_fini')
    words(0xBEEF)

    
# ======== (epilog)
code=[
    ('EXPORT', '_start'),
    ('EXPORT', 'exit'),
    ('EXPORT', '_exit'),
    ('EXPORT', '_exitm'),
    ('EXPORT', '_exitm_msgfunc'),
    ('EXPORT', '__glink_magic_init'),
    ('EXPORT', '__glink_magic_fini'),
    ('CODE', '_start', code0),
    ('CODE', '.callchain', code1),
    ('DATA', '__glink_magic_init', code2, 2, 2),
    ('DATA', '__glink_magic_fini', code3, 2, 2),
    ('IMPORT', 'main') ]

if args.gt1exec != args.e:
    code.append(('IMPORT', args.gt1exec))        # causes map start stub to be included

if not args.no_runtime_bss_initialization:
    code.append(('IMPORT', '_init_bss'))         # causes _init1.c to be included

module(code=code, name='_start.s');


# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
	
