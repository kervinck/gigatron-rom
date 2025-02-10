#include <gigatron/pragma.h>
#include <errno.h>
#include "_stdio.h"

static int _flush(register FILE *fp)
{
	register int (*fptr)(FILE*,int);
	if (fp && fp->_flag) {
		fp->_flag &= 0xff ^ _IOUNGET;
		if (fptr = fp->_v->flush)
			return fptr(fp, 1);
	}
	return 0;
}

int fflush(register FILE *fp)
{
	if (! fp) {
		_swalk(_flush);
		return 0;
	} else if (! fp->_flag) {
		errno = EINVAL;
		return -1;
	} else {
		return _flush(fp);
	}
}
