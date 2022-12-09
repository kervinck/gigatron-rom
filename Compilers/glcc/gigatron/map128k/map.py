
def map_describe():
    print('''  Memory map '128k' targets Gigatrons equipped with a 128k
  board with either a hardware patch or a special ROM that forces the
  framebuffer to reside in banks 0 or 1.

  This map sets up the video buffer in pages 0x82 to 0x8d and selects bank 2.
  Because of either the hardware patch or the special ROM, the pixels are
  located in bank 1 while the vCPU sees bank 2. Program code and data can
  then use the entire 64k addressable by the Gigatron CPU.
''')

# Note: this map compiles a small stub in 0x200 that checks that the
# memory is sufficient. It avoids loading anything in 0x8200-0x8240 to
# avoid overwriting the stub on a 32KB machine.

# ------------size----addr----step----end---- flags (1=nocode, 2=nodata, 4=noheap)
segments = [ (0x00fa, 0x0200, 0x0100, 0x0500, 0),
             (0x0200, 0x0500, None,   None,   0),
             (0x7800, 0x0800, None,   None,   0),
             (0x0200, 0x8000, None,   None,   0),
             (0x7AC0, 0x8240, None,   None,   0) ]

initsp = 0xfffc
libcon = "con1"

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

def map_place(filename,fragnames):
    '''
    Returns a list of additional (PLACE...) directives 
    for file 'filename' with fragments named 'fragnames'.
    '''
    return []

def map_libraries(romtype):
    '''
    Returns a list of extra libraries to scan before the standard ones
    '''
    return [ libcon ]

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
        # Check ram>64k and expansion present
        LD('memSize');BNE('.err')
        LDWI(0x1f8);PEEK();BEQ('.err')
        # Check romtype
        if romtype and romtype >= 0x80:
            LD('romType');ANDI(0xfc);XORI(romtype);BNE('.err')
        elif romtype:
            LD('romType');ANDI(0xfc);SUBI(romtype);BLT('.err')
        # Call _start
        LDWI(v(args.e));CALL(vAC)
        # Run sanitized version of Marcel's smallest program when machine check fails
        label('.err')
        LDW('frameCount');STW(vLR);ANDI(0x7f);BEQ('.err');
        LDW(vLR);DOKE(vPC+1);BRA('.err')

    module(name='_gt1exec.s',
           code=[ ('EXPORT', '_gt1exec'),
                  ('IMPORT', "_map128ksetup"),
                  ('CODE', '_gt1exec', code0) ] )

    debug(f"synthetizing module '_gt1exec.s' at address 0x200")

# Specify an onload function to reorganize the memory
args.onload.insert(0,'_map128ksetup')

# Warn if the rom is not marked as compatible with this map
if not "may_work_with_map128k" in rominfo:
    warning(f"The specified ROM may not support map128k")


# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
