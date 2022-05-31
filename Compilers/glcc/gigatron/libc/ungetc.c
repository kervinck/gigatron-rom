#include "_stdio.h"
#include <errno.h>

extern int ungetc( int c, FILE *fp)
{
	register char *p = fp->_ptr;
	if (fp->_cnt == 0)
		p = fp->_buf + sizeof(fp->_buf);
	if (fp->_base && p == fp->_base->xtra || p == fp->_buf || c < 0)
		return EOF;
	p = p-1;
	fp->_ptr = p;
	fp->_cnt += 1;
	*p = c;
	return c;
}
