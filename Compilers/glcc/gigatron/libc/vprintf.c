#include <stdio.h>
#include <stdarg.h>

int vprintf(register const char *fmt, register va_list ap)
{
	return vfprintf(stdout, fmt, ap);
}	
