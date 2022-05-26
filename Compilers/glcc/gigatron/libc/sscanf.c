#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "_doscan.h"

int sscanf(register const char *str, const char *fmt, ...)
{
	struct _iobuf f;
	register FILE *fp = &f;
	register va_list ap;
	
	memset(fp, 0, sizeof(f));
	fp->_cnt = strlen(str);
	fp->_ptr = (char*)str;
	fp->_flag = _IOFBF|_IOSTR|_IOREAD|_IOEOF;
	va_start(ap, fmt);
	return _doscan(fp, fmt, ap);
	// va_end(ap) is a no-op
}
