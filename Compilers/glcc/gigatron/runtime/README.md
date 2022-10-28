
# Runtime for Gigatron LCC code

These files provide all the routines that are needed to implement
things that are not provided by VCPU or that are simply eating too
much code space.


 * `rt_save.s`: save/restore callee-saved registers
 * `rt_copy.s`: various functions to copy longs, floats, or structs.
 * `rt_mul.s`, `rt_div.s` : multiplication and division for ints (16 bits)
 * `rt_shl.s`, `rt_shr.s` : left and right shifts for ints (16 bits)
 * `rt_ladd.s`, `rt_lmul.s`, `rt_ldiv.s`: arithmetic on longs (32 bits)
 * `rt_lbitops.s`: bitwise operations on longs (32 bits)
 * `rt_lcmp.s` : comparison on longs (32 bits)
 * `rt_lshl.s`, `rt_lshr.s`: left and right shifts on longs (32 bits)
 * `rt_fp.s` : floating point routines (40 bits)

## API

Symbols named `_@_xxxx` are public API. Comments are scarse.
Their function is best understood by looking how they are called
by VCPU pseudo-instuctions defined inside `glink.py`.

As explained in the main [`README.md`](../../README.md) file, these functions
operate entirely using the `[0xc0-0xcf]` block of zero page memory and 
they only use the normal VCPU stack.

The only runtime functions that are not defined here are
`_@_raise_ferr`, `_@_raise_fovf`, and `_@_raise_zdiv` which are used
to raise exceptions for respectively floating point errors, floating
point overflow, and integer division by zero. If the exception is
resumed, these functions do not return, but pop the return address of
the calling function and jump there. These functions are defined in
file `_suppport.s` in the C library.

Symbols named `__@xxxx` are private to the runtime.


## Status

This is complete and passes the test suite.

Improvement opportunities:

 * One should investigate SYS calls to speedup these operations. 
   Sixteen bits multiplication and division are to be provided by at67's new rom.
   
 * The structure copy code (`rt_copy.s`) could benefit from the same level
   of optimization than [`memcpy`](../libc/memcpy.s).  This is hard
   because these functions must have low overhead for small sizes.

