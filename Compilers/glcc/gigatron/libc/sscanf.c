#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "_doscan.h"

static struct _iovec _sscanf_iovec = { _sread };

int sscanf(register const char *str, const char *fmt, ...)
{
	struct _iobuf f;
	register FILE *fp = &f;
	register va_list ap;
	memset(fp, 0, sizeof(f));
	fp->_flag = _IONBF|_IOREAD;
	fp->_v = &_sscanf_iovec;
	fp->_x = (void*)str;
	va_start(ap, fmt);
	return _doscan(fp, fmt, ap);
	// va_end(ap) is a no-op
}
