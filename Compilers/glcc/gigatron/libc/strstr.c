#include <string.h>

char *
strstr(register const char *haystack, const char *needle)
{
	register int n = strlen(needle);
	register int c = needle[0];
	if (! c)
		return (char*)haystack;
	for(; haystack[0]; haystack++)
		if (haystack[0] == c && !strncmp(haystack, needle, n))
			return (char*)haystack;
	return 0;
}
