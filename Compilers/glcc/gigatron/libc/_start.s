
### The rom/ram checking code must work on all cpu

def code0():
    nohop()
    ### _start()
    label('_start');
    # ensure stack alignment with space for argc and argv
    LDI(3);ORW(SP);SUBI(7);STW(SP)
    # call onload functions
    for f in args.onload:
        _CALLJ(f)
    # initialize bss
    if not args.no_runtime_bss_initialization:
        _CALLJ('_init_bss')
    # call init chain
    _CALLJ('_callchain_init')
    # call main
    LDI(0); STW(R8); STW(R9); _CALLJ('main'); STW(R8)
    ### exit()
    label('exit')
    _MOVW(R8,R0)
    # call fini chain
    if args.cpu < 5:
        _CALLJ('_callchain_fini')
    else:
        LDWI('__glink_magic_fini'); CALLI('_callchain')
    _MOVW(R0,R8)
    ### _exit()
    label('_exit')
    _MOVIW(0,R9)
    label('_exitm');
    _MOVW(R8,R0)
    label('_exitm_msgfunc', pc()+1)
    LDWI(0);_BEQ('.halt')  # _exitm_msgfunc is LDWI's argument here
    CALL(vAC)              # arguments in R8 and R9 are already correct
    # If _exitm_msgfunc is zero or returns
    # we just Flash a pixel with a position indicative of the return code
    label('.halt')
    LDWI(0x100);DEEK();ST(R7+1)
    LD(vACH);ADDW(R0);ST(R7)
    label('.loop')
    POKE(R7);ADDW(0x80)
    BRA('.loop')

def code1():
    # subroutine to call a chain of init/fini functions
    nohop()
    if args.cpu < 5:
        label('_callchain_fini')
        LDWI('__glink_magic_fini'); _BRA('_callchain')
    label('_callchain_init')
    LDWI('__glink_magic_init')
    label('_callchain')
    DEEK(); STW(R7); _MOVW(vLR,R6)
    LDWI(0xBEEF);XORW(R7);_BEQ('.callchaindone')
    LDW(R7);_BRA('.callchaintst')
    label('.callchainloop')
    DEEK();CALL(vAC)
    LDI(2);ADDW(R7);DEEK();STW(R7)
    label('.callchaintst')
    _BNE('.callchainloop')
    label('.callchaindone')
    _MOVW(R6,vLR); RET()

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
for f in args.onload:
    code.append( ('IMPORT', f) )                 # causes onload funcs to be included
if not args.no_runtime_bss_initialization:
    code.append(('IMPORT', '_init_bss'))         # causes _init1.c to be included

module(code=code, name='_start.s');


# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
	
