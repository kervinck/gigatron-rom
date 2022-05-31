#include <limits.h>
#include <stdarg.h>
#include <stdio.h>

int sprintf(char *str, const char *format, ...)
{
  int r;
  va_list ap;
  va_start(ap, format);
  r = vsnprintf(str, UINT_MAX, format, ap);
  va_end(ap);
  return r;
}
