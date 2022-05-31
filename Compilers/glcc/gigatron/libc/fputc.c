#include "_stdio.h"

int fputc(register int c, register FILE *fp)
{
	c = putc(c, fp);
	if (c == '\n' && (fp->_flag & _IOLBF) == _IOLBF)
		_fflush(fp);
	return c;
}
