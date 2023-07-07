#include "_stdio.h"

int fputc(int c, register FILE *fp)
{
	if (! _schkwrite(fp))
		if (fp->_v->write(fp, (const char*)&c, 1) > 0)
			return c;
	return EOF;
}
