#include <stdlib.h>
#include <errno.h>

int errno;

int system(const char *cmd)
{
	errno = ENOTSUP;
	return -1;
}
