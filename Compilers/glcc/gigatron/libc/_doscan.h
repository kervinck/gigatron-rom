#ifndef _DOSCAN_INTERNAL
#define _DOSCAN_INTERNAL

#include <errno.h>

#include "_stdlib.h"
#include "_stdio.h"

#define FLG_STAR    1
#define FLG_LONG    2
#define FLG_LEN     8

typedef struct {
	int c;
	FILE *fp;
	int cnt;
	int n;
} doscan_t;

extern int _doscan(FILE*, const char*, __va_list);
extern int _doscan_next(doscan_t *);
extern int _doscan_double(doscan_t *, double *);

#endif
