#include "_stdio.h"
#include <errno.h>

extern int ungetc(int c, FILE *fp)
{
	register int r;
	if (_schkread(fp) && c >= 0)
		if (! ((r = fp->_flag) & _IOUNGET)) {
			fp->_flag = (r | _IOUNGET) & (0xff ^ _IOEOF);
			return fp->_unget = c;
		}
	return EOF;
}
