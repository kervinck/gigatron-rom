#ifndef _DOPRINT_INTERNAL
#define _DOPRINT_INTERNAL

#include <gigatron/libc.h>
#include <gigatron/printf.h>

#include "_stdlib.h"
#include "_stdio.h"


/* print destination */

extern struct _doprint_dst_s {
	int cnt;
	FILE *fp;
	writall_t writall;
} _doprint_dst;

extern void _doprint_putc(int, size_t);
extern void _doprint_puts(const char*, size_t);

/* print field specification */

#define DPR_LEFTJ   1
#define DPR_ZEROJ   2
#define DPR_SGN     4
#define DPR_SPC     8
#define DPR_ALT    16
#define DPR_LONG   32
#define DPR_WIDTH  64
#define DPR_PREC  128

typedef struct doprintspec_s {
	char flags;
	char conv;
	int width;
	int prec;
} doprintspec_t;

/* print routines */

extern void _doprint_num(doprintspec_t*, int, char*);
extern void _doprint_double(doprintspec_t*, __va_list*);
extern void _doprint_long(doprintspec_t*, int, __va_list*);

#endif
