#include "_stdio.h"
#include <errno.h>


void setbuf(FILE *fp, char *buf)
{
	setvbuf(fp, buf, buf ? _IOFBF : _IONBF, BUFSIZ);
}

int setvbuf(register FILE *fp,
	    register char *buf,
	    register int mode,
	    register size_t sz)
{
	register int flag;
	if ((flag = fp->_flag & (0xff ^ _IOLBF ^ _IOUNGET)) &&
	    (mode == _IONBF || mode == _IOFBF || mode == _IOLBF) ) {
		fp->_flag = flag | mode;
		return 0;
	}
	errno = EINVAL;
	return EOF;
}
