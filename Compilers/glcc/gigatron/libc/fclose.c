#include "_stdio.h"
#include <errno.h>

int fclose(FILE *fp)
{
	if (fp->_flag) {
		register int r = _fclose(fp);
		_sfreeiob(fp);
		return r;
	}
	errno = EINVAL;
	return -1;
}
