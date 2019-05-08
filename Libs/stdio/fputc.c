#include <stdio.h>

int fputc(int c, FILE *stream)
{
  // XXX stream ignored
  return putchar(c);
}
