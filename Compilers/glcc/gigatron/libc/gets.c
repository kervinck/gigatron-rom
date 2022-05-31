#include "_stdio.h"

char *gets(char *s)
{
	register int c;
	register char *p = s;
	while ((c = getchar()) != EOF && c != '\n')
		*p++ = c;
	*p = 0;
	if (ferror(stdin))
		return 0;
	if (feof(stdin) && p == s)
		return 0;
	return s;
}
