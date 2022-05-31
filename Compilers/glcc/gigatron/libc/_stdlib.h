#ifndef _STDLIB_INTERNAL
#define _STDLIB_INTERNAL

#include <stdlib.h>


/* Weak references '__glink_weak_xxxx' do not cause anything to be imported. 
   They resolve to 'xxxx' if it is defined and zero otherwise. */

extern void *__glink_weak_malloc(size_t);
extern void __glink_weak_free(void*);


/* String to number conversions */

typedef struct {
	int flags, base;
	unsigned long x;
} strtol_t;

extern int _strtol_push(strtol_t*, int c);
extern int _strtol_decode_u(strtol_t*, unsigned long *px);
extern int _strtol_decode_s(strtol_t*, long *px);

typedef struct {
	int flags;
	int e0, e1;
	double x;
} strtod_t;

extern int _strtod_push(strtod_t*, int c, const char *p);
extern int _strtod_decode(strtod_t*, double *px);

#endif




