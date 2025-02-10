#include <stdarg.h>
#include <stdlib.h>

#include "_stdio.h"
#include "_doprint.h"

int vfprintf(register FILE *fp, register const char *fmt, register __va_list ap)
{
	_doprint_dst.fp = fp;
	if ((_doprint_dst.writall = _schkwrite(fp))) {
		register int c = _doprint(fmt, ap);
		if (!ferror(fp))
			return c;
	}
	return EOF;
}

/* A fprintf relay is defined in _printf.s */
