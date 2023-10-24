#include "_stdio.h"

int fputc(int c, register FILE *fp)
{
	register writall_t fptr;
	if ((fptr = _schkwrite(fp)))
		if (fptr((const char*)&c, 1, fp) == 1)
			return c;
	return EOF;
}
