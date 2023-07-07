#include "_stdio.h"
#include <errno.h>

static int _schkseek(register FILE *fp)
{
	register int flag;
	if (! (flag = fp->_flag))
		return errno = EINVAL;
	if (! fp->_v->lseek)
		return errno = ENOTSUP;
	return 0;
}

long int ftell(FILE *fp)
{
	if (! _schkseek(fp))
		return fp->_v->lseek(fp, 0, SEEK_CUR);
	return EOF;
}

int fseek(FILE *fp, long int off, int whence)
{
	if (_schkseek(fp))
		return EOF;
	if (fp->_v->lseek(fp, off, whence) <= 0)
		return _serror(fp, EOF);
	fp->_flag &= 0xff ^ _IOEOF ^ _IOUNGET;
	return 0;
}

void rewind(FILE *fp)
{
	fseek(fp, 0, SEEK_SET);
	clearerr(fp);
}
