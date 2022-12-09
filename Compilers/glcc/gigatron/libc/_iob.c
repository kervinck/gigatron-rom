#include "_stdio.h"

struct _iobuf _iob[_IOB_NUM] = { 0 };
struct _more_iobuf *_more_iob = 0;

int _schkwrite(register FILE *fp)
{
	register int flag = fp->_flag;
	if (flag & (_IOERR|_IOEOF))
		return EOF;
	if (! (flag & _IOWRIT))
		return errno = EPERM;
	return 0;
}

int _schkread(register FILE *fp)
{
	register int flag = fp->_flag;
	if (flag & (_IOERR|_IOEOF))
		return EOF;
	if (! (flag & _IOREAD))
		return errno = EPERM;
	return 0;
}

int _serror(FILE *fp, int errn)
{
	if (errn > 0) {
		errno = errn;
		fp->_flag |= _IOERR;
	} else if (errn < 0)
		fp->_flag |= _IOEOF;
	return EOF;
}

void _swalk(int(*func)(FILE*))
{
	register FILE *f = _iob;
	register struct _more_iobuf *next = _more_iob;
	for(;;) {
		register char i;
		for (i = 0; i != _IOB_NUM; f++, i++)
			if  (f->_flag)
				(*func)(f);
		if (next) {
			f = next->_iob;
			next = next->next;
		} else
			break;
	}
}

int _fclose(FILE *fp)
{
	int (*flush)(FILE*,int);
	if (fp->_flag && (flush = fp->_v->flush))
		return flush(fp, 1);
	return 0;
}

static void _fcloseall(void)
{
	_swalk(_fclose);
}

DECLARE_INIT_FUNCTION(_iob_setup);
DECLARE_FINI_FUNCTION(_fcloseall);

