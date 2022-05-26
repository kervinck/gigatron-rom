#include "_stdio.h"
#include <errno.h>

FILE *tmpfile(void)
{
	errno = ENOTSUP;
	return 0;
}

char *tmpnam(char *s)
{
	errno = ENOTSUP;
	return 0;
}
