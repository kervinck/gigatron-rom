#include <string.h>
#include <gigatron/libc.h>

size_t strlen(register const char *s)
{
	return (const char*)_memchr2(s, 0, 0, 0xffffu) - s;
}
