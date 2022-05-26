#include <string.h>
#include <gigatron/libc.h>

char *
strncat(register char *dst, register const char *src, register size_t n)
{
	register char *e = _memchr2(dst, 0, 0, 0xffffu);
	register int l = strlen(src);
	if (l > n)
		l = n;
	memcpy(e, src, l);
	e[l] = 0;
	return dst;
}
