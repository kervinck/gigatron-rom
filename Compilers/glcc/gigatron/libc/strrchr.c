#include <string.h>
#include <gigatron/libc.h>

char *
strrchr(register const char *p, register int ch)
{
	register const char *r = 0;
	register const char *q = p;
	while (*q && (q = __memchr2(q, (char)ch, 0xffffu)))
		while (*q && *q == ch) {
			r = q;
			q += 1;
		}
	return (char*)r;
}

