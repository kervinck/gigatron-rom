#include "_stdio.h"

int puts(register const char *s)
{
	fputs(s, stdout);
	putc('\n', stdout);
	if ((stdout->_flag & _IOLBF) == _IOLBF)
		fflush(stdout);
	return _fcheck(stdout);
}
