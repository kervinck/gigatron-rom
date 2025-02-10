#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "_doscan.h"

static int _sread(FILE *fp, register char *buf, size_t cnt)
{
	register int c;
	register char **xptr = (char**)&fp->_x;
	if (! *xptr || !(c = **xptr))
		return 0;
	buf[0] = c;
	*xptr += 1;
	return 1;
}

static struct _iovec _sscanf_iovec = { 0, _sread };

int sscanf(register const char *str, const char *fmt, ...)
{
	struct _iobuf f;
	register FILE *fp = &f;
	register va_list ap;
	fp->_flag = _IONBF|_IOREAD;
	fp->_v = &_sscanf_iovec;
	fp->_x = (void*)str;
	va_start(ap, fmt);
	return _doscan(fp, fmt, ap);
	// va_end(ap) is a no-op
}
