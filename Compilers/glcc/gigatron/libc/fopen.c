#include "_stdio.h"
#include <errno.h>

static int _sflags(int f, const char *s)
{
	char c = *s;
	f = f & _IOBUFMASK; 
	if (c == 'r')
		f |= _IOREAD;
	else if (c == 'w')
		f |= _IOWRIT;
	else if (c == 'a')
		/* _IOEOF means append mode to _open() */
		f |= _IOWRIT|_IOEOF;
	else
		return 0;
	while (c = *++s) {
		if (c == 'b')
			continue;
		else if (c == '+')
			/* no buffering for r/w streams */
			f = f & ~_IOLBF | (_IORW | _IONBF); 
		else
			return 0;
	}
	return f;
}

FILE *freopen(register const char *fname, register const char *mode, register FILE *fp)
{
	register int oflag, nflag;
	if (! (oflag = nflag = fp->_flag))
		nflag = _IOFBF;
	nflag = _sflags(nflag, mode);
	if (! (nflag && fname)) {
		errno = EINVAL;
		return 0;
	}
	if (oflag) {
		_fclose(fp);
	} else {
		fp->_file = -1;
	}
	fp->_flag = nflag;
	if (_openf(fp, fname) >= 0) {
		clearerr(fp);
		return fp;
	}	
	_sfreeiob(fp);
	return 0;
}

FILE *fopen(register const char *fname, register const char *mode)
{
	return freopen(fname, mode, _sfindiob());
}


