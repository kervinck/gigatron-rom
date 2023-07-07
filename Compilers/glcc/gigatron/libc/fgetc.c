#include "_stdio.h"

int fgetc(register FILE *fp) {
	register int r;
	char c;
	if (_schkread(fp)) {
		return EOF;
	}
	if ((r = fp->_flag) & _IOUNGET) {
		fp->_flag = r ^ _IOUNGET;
		return fp->_unget;
	}
	if ((r = fp->_v->read(fp, &c, 1)) <= 0)
		return _serror(fp, (r) ? EIO : EOF);
	return c;
}
