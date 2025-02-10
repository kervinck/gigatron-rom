#ifndef __GIGATRON_PRAGMA
#define __GIGATRON_PRAGMA

/* This file defines syntax constructions whose semantic is specific
   to the gigatron platform. These constructions include declaration
   attributes and pragmas. Convenient definitions are provided
   to cover common use cases and to ensure forward and backward
   compatibility. */

/* ==== Pragmas ====

   The following pragmas are supported:

   * #pragma glcc option("OPTION")
     Passes argument --option=OPTION to the linker.
   * #pragma glcc lib("LIB")
     Passes argument -lLIB to the linker.


   ==== Declaration attributes ====

   The syntax `__attribute__((LIST))`, where `LIST` is a
   comma-separated list of identifiers optionally followed by a comma
   separated list of argumentds in parentheses. The `__attribute__`
   keyword can appear in the declaration specifier (in which case it
   applies to all the declared symbols) or after each declarator (in
   which case it only applies to one symbol).

   Definition attributes follow the glink constraint specifications.
   They can be used in variable definitions and function definitions.

   * `__attribute__((nohop))`
     Variable cannot cross a page boundary.
   * `__attribute__((org(ADDRESS)))
     Variable must be allocated at the specified address
   * `__attribute__((offset(ADDRESS)))
     Variable must be allocated at page offset ADDRESS&0xff.
   * `__attribute__((place(AMIN,AMAX)))`
     Variable must be allocated between addresses `AMIN` and `AMAX`.

   Two attributes are recognized on extern declarations:

   * `__attribute__((alias(STRING)))`
     Define an external variable that references the linker symbol
     `STRING` instead of a symbol whose name is equal to the variable
     name. When `STRING` starts with prefix "__glink_weak_", this is a
     weak reference.
   * `__attribute__((org(ADDRESS)))`
     Define an external variable assumed in the current compilation
     unit to be located at absolute address `ADDRESS`.


   ==== Attribute macros ====

   The following definitions are provided as convenience
   and also to protect programs against possible changes
   in attribute syntax or semantics: */

/*  `__nohop` --
    Mark a variable or a function that should
    not cross page boundaries.
    Example:
    | char sprite[] __nohop = { 0x40, 0x23, ... }; */

#define __nohop __attribute__((nohop))

/* `__lomem` --
   Mark a variable or a function that should be allocated in bank0,
   that is at locations below 0x8000.
   Example:
   | void somebankingwork(int args) __lomem { ... } */

#define __lomem __attribute__((place(0x0200,0x07ff)))

/* `__himem` --
   Mark a variable or a function that should be allocated
   at addresses greater than 0x8000 on a gigatron with at
   least 64k of memory.
   Example:
   | struct foo array[20][20] __himem; */

#define __himem __attribute__((place(0x8000,0xffff)))

/* `__weakref(x)` --
   Causes an external variable to be a weak reference to linker symbol
   `s`.  If no module defines this variable, no compilation error will
   be reported, and the address of the variable will be set to
   zero. */

#define __weakref(x) __attribute__((alias("__glink_weak_" x)))

/* `__weak` --
   Deprecated. */

#define __weak  __attribute__((alias("__glink_weak_%s")))

/* `__at(ADDRESS)` --
   Indicate that a variable lives at a fixed address.
   In a variable definition, the linker will be asked to
   allocate the variable at the specified address.
   In an external variable declaration, the linker will
   resolve the variable with the specified address in
   the current compilation unit.
   Example:
   | extern unsigned char ctrlBits __at(0x1f8); */

#define __at(x) __attribute__((org(x)))


/* `__offset(ADDRESS)` --
   Indicates that the variable should be located
   at an address whose last byte matches ADDRESS.
   Example:
   | extern struct vCpuContext_s context __offset(0xe0); */

#define __offset(x) __attribute__((offset(x)))


/* Note that the keywords `__near` and `__far` are not attributes but
   are type qualifiers comparable to `const` or `volatile`.  They can
   be used in typedefs and inside declarators.  For instance `int *
   __near a;` and `int __near *a` are quite different. The first one
   locates pointer a in page zero, the second one says that pointer a
   points to integers located in page zero. Such subtleties do not
   exist for attributes.  An attribute, even in a typedef, will be
   applied to the declared variables regardless of its position in the
   type specifier or the declarator. */


#endif
