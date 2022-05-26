
#include <string.h>

int
memcmp(const void *s1, const void *s2, size_t n)
{
	register int d;
	while ((d=n) && !(d= *(const unsigned char*)s1 - *(const unsigned char*)s2)) {
		s1 = (const unsigned char*)s1 + 1;
		s2 = (const unsigned char*)s2 + 1;
		n--;
	}		
	return d;
}
