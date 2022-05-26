#include <string.h>
#include "_stdio.h"

size_t fwrite(register const void *buf,
	      register size_t sz, register size_t n,
	      register FILE *fp)
{
	register int r;
	if ((r = _schkwrite(fp)))
		return _serror(fp, n);
	n = _fwrite(fp, buf, n * sz);
	return n / sz;
}

