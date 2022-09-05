#include "_stdio.h"

struct _iobuf _iob[_IOB_NUM] = { 0 };
struct _more_iobuf *_more_iob = 0;

int _schkwrite(register FILE *fp)
{
	register int flag = fp->_flag;
	if (flag & (_IOERR|_IOEOF))
		return EOF;
	if (! (flag & _IOWRIT))
		return EPERM;
	if (! (fp->_v->flsbuf && fp->_v->write))
		return ENOTSUP;
	return 0;
}

int _schkread(register FILE *fp)
{
	register int flag = fp->_flag;
	if (flag & (_IOERR|_IOEOF))
		return EOF;
	if (! (flag & _IOREAD))
		return EPERM;
	if (! (fp->_v->filbuf && fp->_v->read))
		return ENOTSUP;
	return 0;
}

int _serror(FILE *fp, int errn)
{
	if (errn > 0) {
		errno = errn;
		fp->_flag |= _IOERR;
	} else if (errn < 0)
		fp->_flag |= _IOEOF;
	if (errn) {
		fp->_cnt = 0;
		fp->_ptr = 0;
		return EOF;
	} else
		return 0;
}

int _flsbuf(register int c, register FILE *fp)
{
	register int n;
	if ((n = _schkwrite(fp)))
		return _serror(fp, n);
	return fp->_v->flsbuf(c, fp);
}

int _filbuf(register FILE *fp)
{
	register int n;
	register char *buf;
	if ((n = _schkread(fp)))
		return _serror(fp, n);
	return fp->_v->filbuf(fp);
}

int _fcheck(register FILE *fp)
{
	register int f = fp->_flag;
	if (f == 0 || (f & (_IOERR|_IOEOF)))
		return EOF;
	return 0;
}

int _fflush(register FILE *fp)
{
	register int flag;
	flag = fp->_flag;
	if ((flag & (_IOFBF|_IOWRIT)) == (_IOFBF|_IOWRIT)) {
		_flsbuf(EOF, fp);
		return _fcheck(fp);
	} else {
		fp->_cnt = 0;
		return 0;
	}
}

int _fclose(register FILE *fp)
{
	register int r = 0;
	if (_fflush(fp))
		r = -1;
	if (fp->_v && fp->_v->close && (*fp->_v->close)(fp) < 0)
		r = -1;
	return r;
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
		if (! next)
			break;
		f = next->_iob;
		next = next->next;
	}
}

static void _fcloseall(void)
{
	_swalk(_fclose);
}

DECLARE_INIT_FUNCTION(_iob_setup);
DECLARE_FINI_FUNCTION(_fcloseall);

