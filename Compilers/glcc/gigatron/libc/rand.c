#include <stdlib.h>
#include <gigatron/sys.h>

static char init;
static long seed;

void srand(register unsigned int x)
{
	init = 1;
	seed = x;
	rand();
	rand();
}

void _srand(void)
{
	srand(SYS_Random());
}

int rand(void)
{
	if (!init) _srand();
	/* Simple LCG. The multiplier comes from Steele's paper. */
	seed = seed * 0xa13fc965L + 1013904223L;
	return ((int*)&seed)[1] & 0x7fff;
}


