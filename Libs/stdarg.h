#ifndef _VA_LIST
#define _VA_LIST
typedef char *va_list;
#endif

// Function arguments are just words for now, because all goes through
// @pusha and our types are still 1 or 2 bytes long. Except structs, but
// LCC silently converts those to pointers before pushing on the stack
// (by virtue of gt1IR.want_argsb=0 in Utils/lcc/src/gt1.md).
// XXX This means that va_arg() gets it WRONG now for any passed structs!
//     We can use our non-standard va_sarg() as a workaround.

#define _va_sizeof(var)         ((sizeof(var) + 1U) & ~1U)

// Initialize an argument pointer
#define va_start(ap, last)      (void) ((ap) = (va_list)&(last) +\
                                                     _va_sizeof(last))

// Fetch argument and advance argument pointer
#define va_arg(ap, type)        * (type*) ( ((ap) += _va_sizeof(type))\
                                                   - _va_sizeof(type) )

// Use this macro (only) for struct types, for example as follows:
//      struct xy p;
//      p = va_sarg(ap, struct xy);
#define va_sarg(ap, stype)      ** (stype**)( ((ap) += _va_sizeof(stype*))\
                                                     - _va_sizeof(stype*) )

// Optional cleanup up
#define va_end(ap)              ((void) 0)
