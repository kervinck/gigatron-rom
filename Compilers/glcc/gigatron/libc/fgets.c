#include "_stdio.h"

char *fgets(char *s, int sz, FILE *fp)
{
	register int c;
	register char *p = s;
	while (p < s + sz - 1 && ((c = getc(fp)) != EOF)) {
		*p++ = c;
		if (c == '\n')
			break;
	}
	*p = 0;
	if (ferror(fp))
		return 0;
	if (feof(fp) && p == s)
		return 0;
	return s;
}
