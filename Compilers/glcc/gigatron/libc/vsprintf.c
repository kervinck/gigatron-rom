#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "_doprint.h"

typedef struct { char *x; } spdata_t;

static spdata_t spdata;

static int _sprintf_writall(register const char *buf, register size_t sz, register FILE *fp)
{
	register char *b = ((spdata_t*)fp)->x;
	if (b) {
		memcpy(b, buf, sz);
		b += sz;
		*b = 0;
		((spdata_t*)fp)->x = b;
	}
	return sz;
}

int vsprintf(register char *s, register const char *fmt, register va_list ap)
{
	_doprint_dst.writall = (writall_t)_sprintf_writall;
	_doprint_dst.fp = (FILE*)&spdata;
	spdata.x = s;
	return _doprint(fmt, ap);
}

/* A sprintf relay is defined in _printf.s */
