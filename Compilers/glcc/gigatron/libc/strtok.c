#include <string.h>

char *
strtok(register char *str, register const char *delim)
{
	static char *s = 0;
	register char *t;
	if (str)
		s = str;
	t = s + strspn(s, delim);
	if (! *t)
		return NULL;
	s = t + strcspn(t, delim);
	if (*s)
		*s++ = 0;
	return t;
}
