#include <string.h>

void *memmove(void *d, register const void *s, size_t n)
{
	register char *e = (char*)s + n;
	register char *p = (char*)d + n;
	if (d < s || (char*)d >= e)
		return memcpy(d, s, n);
	while (e != s) {
		p -= 1;
		e -= 1;
		*p = *e;
	}
	return d;
}
