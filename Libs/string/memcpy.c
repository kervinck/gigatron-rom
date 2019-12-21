#include <string.h>

void *memcpy(void *dst, const void *src, size_t n)
{
  char *p = dst;
  const char *q = src;

  while (n--)
    *p++ = *q++;

  return dst;
}
