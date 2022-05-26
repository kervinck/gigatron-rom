
#include <string.h>

int
strcmp(const char *s1, const char *s2)
{
	register int d, e;
	while(e=*s2, d=*s1-e, !d && e) {
		s1++;
		s2++;
	}
	return d;
}
