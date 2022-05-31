#include <stdio.h>

/*
 *  Return nonnegative integer on success and EOF on error
 */
int fputs(const char *s, FILE *stream)
{
  for (; *s; s++)
    if (putc(*s, stream) < 0)
      return EOF;
  return fflush(stream);
}
