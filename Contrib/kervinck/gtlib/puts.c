
#include "Gigatron.h"

#include "stdio.h"

/*
 *  Write the given string to stdout, appending a newline.
 *  Returns a nonnegative integer on success and EOF on error.
 */
int puts(const char *s)
{
  // XXX Linker crashes for 'char *ss'
  //const char *ss = (const char *)s;
  const byte *ss = (const byte *)s;
  for (; *ss != '\0'; ss++)
    putc(*ss);
  //Newline();
  return 0;
}

