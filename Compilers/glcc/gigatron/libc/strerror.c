#include <math.h>
#include <errno.h>

char *sys_errlist[] = {
    /* NOERROR  0 */ "Unknown error",
    /* EDOM     1 */ "Domain error",
    /* ERANGE   2 */ "Result too large",
    /* EINVAL   3 */ "Invalid argument",
    /* ENOENT   4 */ "No such file or directory",
    /* ENOTDIR  5 */ "Not a directory",
    /* ENFILE   6 */ "Too many open files",
    /* ENOMEM   7 */ "Not enough memory",
    /* EIO      8 */ "Input/output error",
    /* EPERM    9 */ "Permission denied",
    /* ENOTSUP 10 */ "Not supported"
};

#define NELEMS(a) (sizeof(a)/sizeof(a[0]))

int sys_nerr = NELEMS(sys_errlist);

char *strerror(int n)
{
	if (n >= 0 && n < NELEMS(sys_errlist))
		return sys_errlist[n];
	return sys_errlist[0];
}
