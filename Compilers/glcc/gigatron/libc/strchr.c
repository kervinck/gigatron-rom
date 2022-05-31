#include <string.h>
#include <gigatron/libc.h>

char *
strchr(register const char *p, register int ch)
{
	const char *q = _memchr2(p, ch, 0, 0xffffu);
	if (q && *q)
		return (char*)q;
	return 0;
}

