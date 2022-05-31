#include <ctype.h>

/* Table based ctype does not make sense in the gigatron 
   because it requires too much contiguous memory. */

int (isdigit)(int c)
{
	return _isdigit(c);
}
