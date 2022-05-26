#include <string.h>
#include <gigatron/libc.h>

static unsigned char _mask[8] = { 1,2,4,8,16,32,64,128 };

void
_spnset(register char *b, register const char *set)
{
	register int c;
	_bitset_clear(b, 32);
	while (c = *set) {
		_bitset_set(b, c);
		set++;
	}
}

size_t
strspn(register const char *s0, register const char *set)
{
	char b[32];
	register int c;
	register const char *s = s0;
	_spnset(b, set);
	while ((c = *s0) && _bitset_test(b, c))
		s0 += 1;
	return s0 - s;
}

size_t
strcspn(register const char *s0, register const char *set)
{
	char b[32];
	register int c;
	register const char *s = s0;
	_spnset(b, set);
	while ((c = *s0) && !_bitset_test(b, c))
		s0 += 1;
	return s0 - s;
}
