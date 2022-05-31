#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

int _assert(char *e, char *f, unsigned int l)
{
#if NOT_YET_IMPLEMENTED
	if (e && f)
		fprintf(stderr,"Assertion failed (%s:%d): %s\n", f, l, e);
#endif
	extern int _exitm(int,const char*);
	return _exitm(10, "Assertion failed");
}

#undef assert

void assert(int e)
{
	if (!e) _assert(0,0,0);
}
