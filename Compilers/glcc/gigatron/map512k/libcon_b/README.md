
# LIBCON_B - Banked console library

This library relocates the screen into page 14 and implements a basic
GLCC console. This potentially frees all the 64KB of the normally
mapped banks 0 and 1. The code works by overriding the setup code and
the low-level routines of the normal GLCC console library. It assumes
that the running program does not manipulate the extended banking or
merely expects it to be reset to bank 0 after each console library
call. It can run with the unmodified v5a ROM.
