#include <string.h>

char *
strpbrk(const char *s, const char *accept)
{
	s += strcspn(s, accept);
	if (*s)
		return (char*)s;
	return 0;
}
