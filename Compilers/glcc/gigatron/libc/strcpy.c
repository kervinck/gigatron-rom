#include <string.h>
#include <gigatron/libc.h>

char *
strcpy(register char *dst, register const char *src)
{
	register char *e = __memchr2(src, 0, 0xffffu);
	memcpy(dst, src, e - src + 1);
	return dst;
}
