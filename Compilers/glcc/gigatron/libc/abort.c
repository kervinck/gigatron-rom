#include <stdlib.h>
#include <signal.h>
#include <gigatron/libc.h>

void abort(void)
{
	_raisem(SIGABRT, "Abort");
}
