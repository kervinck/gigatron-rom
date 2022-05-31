#include <stdarg.h>
#include <stdio.h>

int scanf(const char *format, ...)
{
  va_list ap;
  int r;
  va_start(ap, format);
  r = vfscanf(stdin, format, ap);
  va_end(ap);
  return r;
}
