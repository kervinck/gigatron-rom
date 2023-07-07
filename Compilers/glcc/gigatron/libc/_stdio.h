#ifndef _STDIO_INTERNAL
#define _STDIO_INTERNAL

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <gigatron/libc.h>

#include "_stdlib.h"

#define _IOB_NUM 5

extern struct _more_iobuf {
	struct _iobuf _iob[_IOB_NUM];
	struct _more_iobuf *next;
} *_more_iob;

extern int    _schkwrite(FILE*);
extern int    _schkread(FILE*);
extern FILE*  _sfindiob(void);
extern int    _serror(FILE*, int);
extern int    _sread(FILE*, char*, size_t);
extern void   _swalk(int(*f)(FILE*));
extern int    _fclose(FILE*);
extern size_t _fwrite(FILE*, const char*, size_t);

#define CONS_BUFSIZE 80

extern struct _iovec _cons_iovec;

#endif
