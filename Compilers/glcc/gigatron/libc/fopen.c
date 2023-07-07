#include "_stdio.h"
#include <errno.h>

static int _sflags(register const char *s)
{
	register int c,f;
	switch(*s) {
	case 'r': f = _IOREAD; break;
	case 'w': f = _IOWRIT; break;
	case 'a': f = _IOWRIT|_IOEOF; break;
	default: return 0;
	}
	while((c = *++s))
		switch(c) {
		case '+': f = _IORW; break;
		case 'b': break;
		default: return 0;
		}
	return f;
}

FILE *freopen(register const char *fname, register const char *mode, register FILE *fp)
{
	register int nflag;
	if (! (nflag = _sflags(mode))) {
		errno = EINVAL;
		return 0;
	}
	_fclose(fp);
	fp->_flag = nflag;
	if (_openf(fp, fname) >= 0) {
		clearerr(fp);
		return fp;
	}	
	fp->_flag = 0;
	return 0;
}

FILE *fopen(register const char *fname, register const char *mode)
{
	return freopen(fname, mode, _sfindiob());
}


