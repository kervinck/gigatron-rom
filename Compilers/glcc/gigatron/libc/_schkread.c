#include "_stdio.h"

read_t _schkread(register FILE *fp)
{
	register read_t fptr;
	register int *perr = &errno;
	register int flag = fp->_flag;
	if (! flag)
		*perr = EINVAL;
	else if (! (fptr = fp->_v->read))
		*perr = ENOTSUP;
	else if (! (flag & _IOREAD))
		*perr = EPERM;
	else if (! (flag & (_IOERR|_IOEOF)))
		return fptr;
	return 0;
}

