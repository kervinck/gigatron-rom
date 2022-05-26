#include <ctype.h>

/* Table based ctype does not make sense in the gigatron 
   because it requires too much contiguous memory. */

int (isgraph)(int c)
{
	return c - 0x20 > 0 && c - 0x83 <= 0 && c != 0x7f;
}

int (isprint)(int c)
{
	return c - 0x20 >= 0 && c - 0x83 <= 0 && c != 0x7f;
}
