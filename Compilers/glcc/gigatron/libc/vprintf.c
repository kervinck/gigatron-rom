#include <stdio.h>
#include <stdarg.h>

#include "_stdio.h"
#include "_doprint.h"

int vprintf(register const char *fmt, register __va_list ap)
{
	_doprint_dst.fp = stdout;
	if ((_doprint_dst.writall = _schkwrite(stdout)))
		return _doprint(fmt, ap);
	return EOF;
}

/* A printf relay is defined in _printf.s */
