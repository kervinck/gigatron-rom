
#include <Gigatron.h>
#include <stdio.h>

/*
 *  Write the given string to stdout and appending a newline.
 *  Returns a nonnegative integer on success and EOF on error.
 */
int puts(const char *s)
{
  // 2019-04-25 (marcelk) XXX Workaround missing asm.sext()
  const byte *t = (const byte*) s;

  for (; *t; t++)
    putchar(*t);
  Newline();

  return 0;
}

