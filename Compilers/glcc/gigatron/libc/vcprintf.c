#include <stdarg.h>
#include <stdlib.h>
#include <gigatron/console.h>

#include "_doprint.h"

int vcprintf(const char *fmt, register __va_list ap)
{
	_doprint_dst.writall = (writall_t)console_writall;
	return _doprint(fmt, ap);
}

/* A cprintf relay is defined in _printf.s */
