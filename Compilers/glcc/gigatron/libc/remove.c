#include "_stdio.h"
#include <errno.h>

int remove(const char *s)
{
	errno = ENOTSUP;
	return -1;
}
