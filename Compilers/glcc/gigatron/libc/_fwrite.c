#include <string.h>
#include "_stdio.h"

size_t _fwrite(register FILE *fp, register const char *buf, register size_t sz)
{
	if (sz <= fp->_cnt) {
		memcpy(fp->_ptr, buf, sz);
		fp->_ptr += sz;
		fp->_cnt -= sz;
		return sz;
	} else {
		register int n;
		register size_t written = 0;
		register int (*write)(FILE*,const void*,size_t) = fp->_v->write;
		_fflush(fp);
		while (written < sz) {
			n = (*write)(fp, buf + written, sz - written);
			if  (n <= 0) { _serror(fp, EIO); break; }
			written += n;
		}
		return written;
	}
}
