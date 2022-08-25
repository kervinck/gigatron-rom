#include "_stdio.h"

int fputc(register int c, register FILE *fp)
{
	return putc(c, fp);
}
