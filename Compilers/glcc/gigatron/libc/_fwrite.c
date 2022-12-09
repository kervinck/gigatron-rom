#include "_stdio.h"

size_t _fwrite(register FILE *fp, register const char *buf, register size_t sz)
{
	register int n;
	register size_t written = 0;
	register int (*write)(FILE*,const char*,size_t) = fp->_v->write;
	if (_schkwrite(fp))
		return 0;
	while (written < sz) {
		n = (*write)(fp, buf + written, sz - written);
		if  (n <= 0) {
			_serror(fp, EIO);
			break;
		}
		written += n;
	}
	return written;
}
