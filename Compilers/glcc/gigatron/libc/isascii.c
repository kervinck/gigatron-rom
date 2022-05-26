#include <ctype.h>

/* Table based ctype does not make sense in the gigatron 
   because it requires too much contiguous memory. */

int (isascii)(int c)
{
	return isascii(c);
}

int (isalnum)(int c)
{
	return _isdigit(c) || _isalpha(c);
}
	
int (iscntrl)(int c)
{
	return c == 0x7f || c >= 0 && c - 0x20 < 0;
}

int (ispunct)(int c)
{
	return c - 0x20 > 0 && c - 0x7f < 0
		&& ! _isdigit(c) && ! _isalpha(c);
}

