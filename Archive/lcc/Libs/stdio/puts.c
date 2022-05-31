#include <stdio.h>

/*
 *  Write the given string to stdout and appending a newline.
 *  Returns a nonnegative integer on success and EOF on error.
 */
int puts(const char *s)
{
  for (; *s; s++)
    if (putc(*s, stdout) < 0)
      return EOF;
  if (putc('\n', stdout) < 0)
    return EOF;
  return fflush(stdout);
}
