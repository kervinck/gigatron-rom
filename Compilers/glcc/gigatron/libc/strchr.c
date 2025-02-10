#include <string.h>
#include <gigatron/libc.h>

char *
strchr(register const char *p, register int ch)
{
	const char *q;
	if (( q = __memchr2(p, (char)ch, 0xffffu)) && (*q))
		return (char*)q;
	return 0;
}

