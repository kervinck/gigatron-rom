#include <stdarg.h>
#include <stdlib.h>
#include <gigatron/console.h>

#include "_doprint.h"


static void dpf(void *closure, const char *buf, size_t sz)
{
	console_print(buf, sz);
}

int vcprintf(const char *fmt, __va_list ap)
{
	doprint_t ddobj;
	register doprint_t *dp = &ddobj;
	dp->cnt = 0;
	dp->f = dpf;
	return _doprint(&ddobj, fmt, ap);
}

int cprintf(const char *fmt, ...)
{
	register va_list ap;
	va_start(ap, fmt);
	return vcprintf(fmt, ap);
}
