#include "_stdio.h"

/* Read from data pointed by _x */

int _sread(FILE *fp, char *buf, size_t cnt)
{
	register char *p = fp->_x;
	register nread = 0;
	while (nread < cnt && p && *p) {
		buf[nread] = *p;
		nread += 1;
		p += 1;
	}
	fp->_x = p;
	return nread;
}

