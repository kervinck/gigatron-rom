#include "_stdio.h"

writall_t _schkwrite(register FILE *fp)
{
	register writall_t fptr;
	register int *perr = &errno;
	register int flag = fp->_flag;
	if (! flag)
		*perr = EINVAL;
	else if (! (fptr = fp->_v->writall))
		*perr = ENOTSUP;
	else if (! (flag & _IOWRIT))
		*perr = EPERM;
	else if (! (flag & (_IOERR|_IOEOF)))
		return fptr;
	return 0;
}
