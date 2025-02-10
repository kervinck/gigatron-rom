#include <string.h>
#include "_stdio.h"


size_t _fread(register FILE *fp, register char *buf, register size_t sz)
{
	register int n;
	register size_t nread = 0;
	register read_t fptr;
	
	if ((fptr = _schkread(fp)) && (sz > 0)) {
		if ((n = fp->_flag) & _IOUNGET) {
			fp->_flag = n ^ _IOUNGET;
			*buf = fp->_unget;
			nread += 1;
		}
		while ((nread < sz) && (n = fptr(fp, buf+nread, sz-nread)) > 0)
			nread += n;
	}
	return nread;
}

size_t fread(register void *buf,
	     register size_t sz, register size_t n,
	     register FILE *fp)
{
	return _fread(fp, buf, n * sz) / sz;
}

