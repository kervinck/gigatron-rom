#include <stdlib.h>
#include <string.h>

extern int __chk_block_header(void *);

void *realloc(register void *ptr, register size_t nsz)
{
	register void *nptr;
	register int osz = 0;
	// check previous block.
	// note that __chk_block_header returns int not uint
	if (ptr && (osz = __chk_block_header((char*)ptr - 8) - 8) < 0)
		return 0;
	if (!(nptr = malloc(nsz)))
		return 0;
	if ((size_t)osz < nsz)
		nsz = osz;
	if (nsz)
		memcpy(nptr, ptr, nsz);
	free(ptr); // free(0) is ok.
	return nptr;
}
