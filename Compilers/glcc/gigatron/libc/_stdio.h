#ifndef _STDIO_INTERNAL
#define _STDIO_INTERNAL

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <gigatron/libc.h>

#include "_stdlib.h"


/* _iob.c */

#define IOBSIZE 4

extern int   _sclose(FILE*);
extern void  _swalk(int(*)(FILE*));
extern FILE* _sfindiob(void);

/* _schkwrite.c */

typedef int (*writall_t)(const char*, size_t, FILE*);
extern writall_t _schkwrite(FILE*);

/* _schkread.c */

typedef int (*read_t)(FILE*, char*, size_t); 
extern read_t _schkread(FILE*);


#endif
