#include <string.h>
#include "_stdio.h"


size_t _fread(register FILE *fp, register char *buf, register size_t sz)
{
	register size_t total = 0;
	register int (*read)(FILE*,void*,size_t) = fp->_v->read;
	if (fp->_cnt >= 0) {
		total = fp->_cnt;
		if (total > sz)
			total = sz;
		memcpy(buf, fp->_ptr, total);
		fp->_cnt = 0;
		fp->_ptr = 0;
		buf += total;
	}
	while (total < sz) {
		register int n;
		if ((n = (*read)(fp, buf, sz - total)) <= 0) {
			_serror(fp, (n) ? EIO : EOF);
			break;
		}
		total += n;
		buf += n;
	}
	return total;
}

size_t fread(register void *buf,
	     register size_t sz, register size_t n,
	     register FILE *fp)
{
	register int r;
	if ((r = _schkread(fp)))
		return _serror(fp, n);
	n = _fread(fp, buf, n * sz);
	return n / sz;
}

