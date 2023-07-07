#include <string.h>
#include "_stdio.h"

size_t fwrite(register const void *buf,
	      register size_t sz, register size_t n,
	      register FILE *fp)
{
	return _fwrite(fp, buf, n * sz) / sz;
}

