#include <string.h>
#include "_stdio.h"

int fputs(register const char *s, register FILE *fp)
{
	register writall_t fptr;
	register size_t len = strlen(s);
	if ((fptr = _schkwrite(fp)))
		if (fptr(s, len, fp) == len)
			return 0;
	return EOF;
}

