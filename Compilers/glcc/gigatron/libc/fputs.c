#include <string.h>
#include "_stdio.h"

int fputs(register const char *s, register FILE *fp)
{
	register int n = _fwrite(fp, s, strlen(s));
	if (ferror(fp))
		return EOF;
	return n;
}

