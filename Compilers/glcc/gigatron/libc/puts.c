#include "_stdio.h"

int puts(register const char *s)
{
	fputs(s, stdout);
	return putc('\n', stdout);
}
