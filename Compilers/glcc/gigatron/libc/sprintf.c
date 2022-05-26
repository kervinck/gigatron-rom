#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "_doprint.h"

typedef struct doprint2_s {
	doprint_t dp;
	char *buffer;
} doprint2_t;

static void dpf(char **bb, const char *buf, size_t sz)
{
	register char *b = *bb;
	if (b) {
		memcpy(b, buf, sz);
		b += sz;
		*b = 0;
		*bb = b;
	}
}

int vsprintf(register char *s, register const char *fmt, register va_list ap)
{
	doprint2_t dp2obj;
	register doprint_t *dp = &dp2obj.dp;
	register char **bb = &dp2obj.buffer;
	dp->cnt = 0;
	dp->closure = bb;
	dp->f = (void(*)(void*,const char*,size_t))dpf;
	*bb = s;
	return _doprint(dp, fmt, ap);
}

int sprintf(register char *s, const char *fmt, ...)
{
	register va_list ap;
	va_start(ap, fmt);
	return vsprintf(s, fmt, ap);
	// va_end(ap) is a no-op
}
