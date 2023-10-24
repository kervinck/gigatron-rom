#include <string.h>
#include <gigatron/libc.h>

char *
strncpy(register char *dst, register const char *src, size_t n)
{
	register int l = (char*)__memchr2(src, 0, 0xffffu) - src;
	if (l > n)
		l = n;
	memcpy(dst, src, l);
	if (l < n)
		memset(dst+l, 0, n-l);
	return dst;
}
