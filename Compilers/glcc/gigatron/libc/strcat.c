#include <string.h>
#include <gigatron/libc.h>

char *
strcat(register char *dst, register const char *src)
{
	register char *e = __memchr2(dst, 0, 0xffffu);
	strcpy(e, src);
	return dst;
}
