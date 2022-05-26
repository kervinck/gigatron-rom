#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include "_doscan.h"

int fscanf(register FILE *fp, const char *fmt, ...)
{
	register va_list ap;
	va_start(ap, fmt);
	return _doscan(fp, fmt, ap);
	// va_end(ap) is a no-op
}

int scanf(const char *fmt, ...)
{
	register va_list ap;
	va_start(ap, fmt);
	return _doscan(stdin, fmt, ap);
	// va_end(ap) is a no-op
}
