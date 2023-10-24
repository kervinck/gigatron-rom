#include "_stdio.h"

int fgetc(register FILE *fp) {
	static char c;
	register int r = fp->_flag;
	register read_t fptr;
	if ((fptr = _schkread(fp))) {
		if (r & _IOUNGET) {
			fp->_flag = r ^ _IOUNGET;
			return fp->_unget;
		} else if (fptr(fp, &c, 1) > 0)
			return c;
	}
	return EOF;
}
