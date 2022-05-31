#include <ctype.h>

/* Table based ctype does not make sense in the gigatron 
   because it requires too much contiguous memory. */

int (isspace)(int c)
{
	return _isspace(c);
}

