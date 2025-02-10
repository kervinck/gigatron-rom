#ifndef __STDBOOL
#define __STDBOOL

/* This is not C99 compliant because lcc does not know _Bool.
   Yet this header can make some programs work.
*/

typedef unsigned int __bool;

#define bool   __bool 
#define true  (1)
#define false (0)
#define __bool_true_false_are_defined 1

#endif
