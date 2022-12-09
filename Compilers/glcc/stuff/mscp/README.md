
# Marcel's Simple Chess Program

MSCP, Marcel's Simple Chess Program by Marcel van Kervinck, is a
small, simple, yet complete open source chess engine released under
the GNU GPL. This version has been adapted to work on the Gigatron
which is another brainchild of Marcel.

https://www.chessprogramming.org/MSCP


## Changes

The main change relocates the `union core` structure containing the
compiled opening book and the transposition table into bank 3 of the
128k gigatron. To that effect, the file `mscp.ovl` ensures that all
objects defined in file `core.c` are placed below 0x7fff and therefore
remain accessible when the banks are switched.  The random generator
has been changed to a subtractive generator that avoids costly long
multiplications.

The opening book is appended to the gt1 at addresses 0xc000 and up.
An onload function defined by `mscp.ovl` is called just after loading
the augmented gt1 file. This function retrieves the book size from
address `0xbffe` and copies the book data into bank3.

A second onload function defined by `map128k` then copies excess code
and data from bank1 to bank2, initializes the framebuffer in bank1,
and switches to bank2. This is how `map128k` provides a contiguous
62KB of memory.
