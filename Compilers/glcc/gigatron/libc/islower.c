#include <ctype.h>

/* Table based ctype does not make sense in the gigatron 
   because it requires too much contiguous memory. */

int (islower)(int c)
{
	return _islower(c);
}

int (isupper)(int c)
{
	return _isupper(c);
}

int (toupper)(register int c) {
	if (_islower(c))
		c = c ^ 0x20;
	return c;
}

int (tolower)(register int c) {
	if (_isupper(c))
		c = c ^ 0x20;
	return c;
}

