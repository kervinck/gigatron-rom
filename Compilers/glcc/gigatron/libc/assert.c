#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <signal.h>
#include <gigatron/libc.h>

int _assertfail(const char *msg)
{
	_raisem(SIGABRT, msg);
	return 0;
}

#undef assert

void assert(int e)
{
	if (!e) _assertfail(0);
}
