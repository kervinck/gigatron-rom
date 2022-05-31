#include <stdio.h>

#ifdef __gigatron__
# define dword long
#else
# define dword int
#endif


dword rand()
{
	static dword whatever = 0;
	whatever = whatever * (dword)1664525L + (dword)1013904223L;
	return whatever;
}

int main()
{
	int i;
	for (i=0; i<100; i++)
		printf("%ld\n", (long)rand());
	return 0;
}
