#include <stdio.h>
#include <stdarg.h>

int fprintf(register FILE *fp, const char *fmt, ...)
{
	register va_list ap;
	va_start(ap, fmt);
	return vfprintf(fp, fmt, ap);
	// va_end(ap) is a noop
}
