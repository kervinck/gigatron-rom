#include <stdlib.h>
#include <gigatron/libc.h>

static unsigned char _mask[8] = { 1,2,4,8,16,32,64,128 };

void _bitset_clear(char *set, size_t sz)
{
	register int i;
	for (i = 0; i != sz; i++)
		set[i] = 0;
}

void _bitset_compl(char *set, size_t sz)
{
	register int i;
	for (i = 0; i != sz; i++)
		set[i] ^= 0xff;
}

void (_bitset_set)(register char *set, register unsigned int i)
{
	register char *p = &set[i>>3];
	register char m = _mask[i&7];
	*p |= m;
}

void (_bitset_clr)(register char *set, register unsigned int i)
{
	register char *p = &set[i>>3];
	register char m = _mask[i&7];
	*p &= m ^ 0xff;
}

int (_bitset_test)(register char *set, register unsigned int i)
{
	register char *p = &set[i>>3];
	register char m = _mask[i&7];
	return *p & m;
}
