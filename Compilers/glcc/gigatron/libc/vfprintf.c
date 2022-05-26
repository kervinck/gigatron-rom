#include <stdarg.h>
#include <stdlib.h>

#include "_stdio.h"
#include "_doprint.h"

int vfprintf(register FILE *fp, register const char *fmt, __va_list ap)
{
	int c;
	doprint_t ddobj;
	
	ddobj.f = (void(*)(void*,const char*,size_t))_fwrite;
	ddobj.closure = fp;
	ddobj.cnt = 0;
	if ((c = _schkwrite(fp)))
		return _serror(fp, c);
	c = _doprint(&ddobj, fmt, ap);
	if (ferror(fp))
		return EOF;
	return c;
}
