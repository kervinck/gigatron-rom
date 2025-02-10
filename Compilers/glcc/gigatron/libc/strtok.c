#include <string.h>

char *
_strtok(register char *str, register const char *delim, register char **ps)
{
	register char *t;
	register char *s;
	if (str)
		*ps = str;
	s = *ps;
	t = s + strspn(s, delim);
	if (! *t)
		return NULL;
	s = t + strcspn(t, delim);
	if (*s)
		*s++ = 0;
	*ps = s;
	return t;
}

char *
strtok(register char *str, register const char *delim)
{
	static char *s = 0;
	return _strtok(str, delim, &s);
}
