
#include <string.h>

int
strncmp(const char *s1, const char *s2, size_t n)
{
	register int d, e;
	while((d=n) && (e=*s2, d=*s1-e, !d && e)) {
		s1++;
		s2++;
		n--;
	}
	return d;
}
