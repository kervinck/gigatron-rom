#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "_doprint.h"

typedef struct { char *x, *e; } spdata_t;

static spdata_t spdata;

static int _sprintf_writall(register const char *buf, register size_t sz, register FILE *fp)
{
	register char *b = ((spdata_t*)fp)->x;
	if (b) {
		register size_t rsz = (size_t)(((spdata_t*)fp)->e - b);
		if (rsz > sz)
			rsz = sz;
		memcpy(b, buf, rsz);
		b += rsz;
		*b = 0;
		((spdata_t*)fp)->x = b;
	}
	return sz;
}

int vsnprintf(register char *s, size_t n, register const char *fmt, register va_list ap)
{
	if (n == 0)
		s = 0;
	_doprint_dst.writall = (writall_t)_sprintf_writall;
	_doprint_dst.fp = (FILE*)&spdata;
	spdata.x = s;
	spdata.e = s + n - 1;
	return _doprint(fmt, ap);
}

/* A snprintf relay is defined in _printf.s */
