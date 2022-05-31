#include <string.h>
#include "_stdio.h"

int fputs(register const char *s, register FILE *fp)
{
	register int n;
	if ((n = _schkwrite(fp)))
		return _serror(fp, n);
	_fwrite(fp, s, strlen(s));
	return _fcheck(fp);
}

