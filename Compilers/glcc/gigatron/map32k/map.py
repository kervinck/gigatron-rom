

def map_describe():
    print('''  Memory map '32k' targets the 32KB Gigatron.
             
  Program and data are scattered in the video memory holes starting in
  0x8a0-0x8ff and progressing towards 0x7fa0-0x7fff. Data items larger
  than 96 bytes can be located in page 2 to 5. The stack grows
  downwards from 0x6fe. This memory map is very constraining because
  it only provides space for a couple data items larger than 96
  bytes. Problems can arise if the stack grows into a data region.

  Overlay 'nochan' can be used to overwrite the memory areas reserved
  to sound channels 2, 3, and 4. Doing so provides a contiguous memory
  area from 0x200 to 0x6ff.

  Overlay 'bare' is meant for small programs that do not use libc,
  do not need libc's initialization and cleanup. These programs
  are placed in the low pages (0x200-0x5ff) in priority.
    ''')


# ------------size----addr----step----end---- flags (1=nocode, 2=nodata, 4=noheap)
segments = [ (0x0060, 0x08a0, 0x0100, 0x80a0, 0),
	     (0x00fa, 0x0200, 0x0100, 0x0500, 1),
	     (0x0100, 0x0500, None,   None,   1)   ]

initsp = 0x6fc
minram = 0x80
nochan = False

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
        # Clears channel mask
        if nochan:
            LD('channelMask_v4');ANDI(0xf8);ST('channelMask_v4')
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
