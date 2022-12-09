#include "_stdio.h"
#include <errno.h>

int fflush(FILE *fp)
{
	register int (*flush)(FILE*,int);
	if (! fp) {
		_swalk(fflush);
		return 0;
	}
	if (! fp->_flag) {
		errno = EINVAL;
		return -1;
	}
	fp->_flag &= 0xff ^ _IOUNGET;
	if ((flush = fp->_v->flush))
		return flush(fp, 0);
	return 0;
}
