#include <stdlib.h>
#include <string.h>

void *calloc(register size_t n, register size_t m)
{
	register size_t sz = n * m;
	register void *ptr;
	if ((ptr = malloc(sz)))
		memset(ptr, 0, sz);
	return ptr;
}
