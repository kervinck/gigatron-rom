#include <string.h>
#include "_stdio.h"

size_t fwrite(register const void *buf,
	      register size_t sz, register size_t n,
	      register FILE *fp)
{
	register writall_t fptr;
	if ((fptr = _schkwrite(fp))) {
		register size_t nsz = sz * n;
		if (fptr(buf, nsz, fp) == nsz)
			return n;
	}
	return 0;
}

