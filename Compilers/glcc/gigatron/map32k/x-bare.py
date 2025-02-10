# This overlay is intended for small programs that do not use libc
# and do not need libc's initialization and cleanup. These programs
# are placed in the low pages (0x200-0x5ff) in priority.


# ------------size----addr----step----end---- flags (1=nocode, 2=nodata, 4=noheap)
segments = [ (0x00fa, 0x0200, 0x0100, 0x0500, 0),
             (0x0100, 0x0500, None,   None,   0),
	     (0x0060, 0x08a0, 0x0100, 0x80a0, 0)   ]

# directly enter 'main' without going through start
args.e = 'main'

# do not rely on bss runtime initialization.
args.no_runtime_bss_initialization = True

# we still want gt1exec to set SP
args.r.append('_gt1exec')

# Do not implicitly link libc.
# Note that adding -lc to the command line might still be necessary
# in order to import c runtime routines, but this has to be intentional.
if len(args.l) > 0 and args.l[0] == 'c':
    args.l = args.l[1:]

# minimal gt1exec only sets SP
def map_modules(romtype):
    debug(f"synthetizing module '_gt1exec.s' at address 0x200")
    def code0():
        org(0x200)
        label('_gt1exec')
        LDWI(initsp);STW(SP);
        LDWI(v(args.e));CALL(vAC)
        label('exit')
        label('_exit')
        label('_exitm')
        label('_exitm_msgfunc',T0)
        HALT()

    module(name='_gt1exec.s',
           code=[ ('EXPORT', '_gt1exec'),
                  ('EXPORT', 'exit'),
                  ('EXPORT', '_exit'),
                  ('EXPORT', '_exitm'),
                  ('EXPORT', '_exitm_msgfunc'),
                  ('CODE', '_gt1exec', code0) ] )

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
