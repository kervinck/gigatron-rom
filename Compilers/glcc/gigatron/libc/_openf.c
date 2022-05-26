#include "_stdio.h"

int _openf(FILE *fp, const char *fname)
{
	errno = ENOTSUP;
	return -1;
}

