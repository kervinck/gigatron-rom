#include <stdarg.h>
#include <stdlib.h>
#include <gigatron/console.h>

#include "_doprint.h"

int midcprintf(const char *fmt, ...)
{
	register va_list ap;
	va_start(ap, fmt);
	_doprint_dst.writall = (writall_t)console_writall;
	return _doprint_simple(fmt, ap);
}
