#include <stdio.h>
#include <Gigatron.h>

int putchar(int c)
{
  return putc(c, stdout);
}
