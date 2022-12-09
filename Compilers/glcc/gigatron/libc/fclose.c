#include "_stdio.h"
#include <errno.h>

int fclose(FILE *fp)
{
	register int r;
	if (! fp->_flag) {
		errno = EINVAL;
		return -1;
	}
	r = _fclose(fp);
	fp->_flag = 0;
	return r;
}
