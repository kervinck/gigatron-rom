#include "_stdio.h"

char *gets(char *s)
{
	register int c;
	register char *p = s;
	register FILE *fp = stdin;
	while ((c = fgetc(fp)) != EOF && c != '\n')
		*p++ = c;
	*p = 0;
	if (ferror(fp))
		return 0;
	if (feof(fp) && p == s)
		return 0;
	return s;
}
