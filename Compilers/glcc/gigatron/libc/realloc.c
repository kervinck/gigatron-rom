#include <stdlib.h>
#include <string.h>

extern int __chk_block_header(void *);

void *realloc(register void *ptr, register size_t nsz)
{
	register size_t osz;
	register void *nptr;
	
	osz = 0;
	if (ptr && !(osz = __chk_block_header((char*)ptr - 6) - 6))
		return 0;
	if (!(nptr = malloc(nsz)))
		return 0;
	if (osz < nsz)
		nsz = osz;
	if (nsz)
		memcpy(nptr, ptr, nsz);
	if (ptr)
		free(ptr);
	return nptr;
}
