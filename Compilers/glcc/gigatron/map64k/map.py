
def map_describe():
    print('''  Memory map '64k' targets memory expanded Gigatrons.
             
 Code and data can be placed in the video memory holes or in the
 extension memory. The stack grows downwards from 0xfffe. Both code
 and small data objects often fit in the memory holes. Meanwhile the
 high 32KB of memory provides space for large data objects.

 Option '--short-function-size-threshold=256' has the effect of using
 high memory for all functions that fit in a page but do not fit in a
 video memory hole. Option '--long-function-segment-size=128' has the
 effect of moving all long functions in high memory, minimizing the
 need to hop from page to page inside the same function. Function
 placement can be seen with glink option '-d' or glcc option '-Wl-d'.
 Overlay 'hionly' can be used to place all the code in the high
 memory 0x8000-0xffff with the only exception of the start stub
 at 0x200.
 ''')

# Note: this map compiles a small stub in 0x200 that checks that the
# memory is sufficient. It avoids loading anything in 0x8200-0x8240 to
# avoid overwriting the stub on a 32KB machine.

# ------------size----addr----step----end---- flags (1=nocode, 2=nodata, 4=noheap)
segments = [ (0x0060, 0x08a0, 0x0100, 0x80a0, 0),
             (0x00fa, 0x0200, 0x0100, 0x0500, 0),
             (0x0200, 0x0500, None,   None,   0),
             (0x0100, 0x8100, None,   None,   0),
             (0x79c0, 0x8240, None,   None,   0)   ]

initsp = 0xfffc
minram = 0x100

def map_segments():
    '''
    Enumerate all segments as tuples (saddr, eaddr, dataonly)
    '''
    global segments
    for tp in segments:
        estep = tp[2] or 1
        eaddr = tp[3] or (tp[1] + estep)
        for addr in range(tp[1], eaddr, estep):
            yield (addr, addr+tp[0], tp[4])

def map_libraries(romtype):
    '''
    Returns a list of extra libraries to scan before the standard ones
    '''
    return []

def map_modules(romtype):
    '''
    Generate an extra modules for this map. At the minimum this should
    define a function '_gt1exec' that sets the stack pointer,
    checks the rom and ram size, then calls v(args.e). This is often
    pinned at address 0x200.
    '''
    def code0():
        org(0x200)
        label(args.gt1exec)
        # Set stack
        LDWI(initsp);STW(SP);
        # Check rom and ram
        if romtype and romtype >= 0x80:
            LD('romType');ANDI(0xfc);XORI(romtype);BNE('.err')
        elif romtype:
            LD('romType');ANDI(0xfc);SUBI(romtype);BLT('.err')
        if minram == 0x100:
            LD('memSize');BNE('.err')
        else:
            LD('memSize');SUBI(1);ANDI(0xff);SUBI(minram-1);BLT('.err')
        # Call _start
        LDWI(v(args.e));CALL(vAC)
        # Run sanitized version of Marcel's smallest program when machine check fails
        label('.err')
        LDW('frameCount');STW(vLR);ANDI(0x7f);BEQ('.err');
        LDW(vLR);DOKE(vPC+1);BRA('.err')

    module(name='_gt1exec.s',
           code=[ ('EXPORT', '_gt1exec'),
                  ('CODE', '_gt1exec', code0) ] )

    debug(f"synthetizing module '_gt1exec.s' at address 0x200")


# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
