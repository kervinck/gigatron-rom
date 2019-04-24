
#include "../Gigatron.h"
#include "../stdio.h"

/*
 *  Write the given string to stdout, appending a newline.
 *  Returns a nonnegative integer on success and EOF on error.
 */
int puts(const char *s)
{
  const char *t = s;
  for (; *t; t++)
    putc(*t);
  Newline();
  return 0;
}

