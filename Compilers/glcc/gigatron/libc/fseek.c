#include "_stdio.h"
#include <errno.h>

static int _schkseek(register FILE *fp)
{
	register int flag;
	if (! (flag = fp->_flag))
		return EINVAL;
	if (flag & (_IOERR|_IOEOF))
		return EOF;
	if (! fp->_v->lseek)
		return ENOTSUP;
	return 0;
}

long int ftell(FILE *fp)
{
	long int off;
	register int r;
	if ((r = _schkseek(fp)))
		return r;
	if ((fp->_flag & _IOWRIT) && _fflush(fp) < 0)
		return EOF;
	if ((off = fp->_v->lseek(fp, 0, SEEK_CUR)) >= 0)
		off -= fp->_cnt;
	return off;
}

int fseek(FILE *fp, long int off, int whence)
{
	register int r;
	if (!(r = _schkseek(fp))) {
		if ((fp->_flag & _IOREAD) && whence == SEEK_CUR)
			off -= fp->_cnt;
		if (_fflush(fp) < 0)
			return EOF;
		if (fp->_v->lseek(fp, off, whence) < 0)
			r = errno;
		fp->_flag &= (_IOEOF ^ 0xff);
	}
	return _serror(fp, r);
}

void rewind(FILE *fp)
{
	if (fseek(fp, 0, SEEK_SET) >= 0)
		clearerr(fp);
}
