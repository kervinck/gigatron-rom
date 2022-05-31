#include <stdio.h>
#include <stdarg.h>

int printf(const char *fmt, ...)
{
	register va_list ap;
	va_start(ap, fmt);
	return vfprintf(stdout, fmt, ap);
	// va_end(ap) is a noop
}
