#include <stdio.h>

/*
 *  Write the given string to stdout and appending a newline.
 *  Returns a nonnegative integer on success and EOF on error.
 */
int puts(const char *s)
{
  for (; *s; s++)
    putchar(*s);
  putchar('\n');
  return 0;
}
