#include <string.h>

void *memset(void *b, int c, size_t len)
{
  unsigned char *p = b;

  for (; len; len--)
    *p++ = c;

  return b;
}
