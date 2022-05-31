#include "_stdio.h"
#include <errno.h>

int rename(const char *s1, const char *s2)
{
	errno = ENOTSUP;
	return -1;
}
