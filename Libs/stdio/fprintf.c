#include <limits.h>
#include <stdarg.h>
#include <stdio.h>

int fprintf(FILE *stream, const char *format, ...)
{
  int r;
  va_list ap;
  va_start(ap, format);
  r = vsnprintf((char*)stream, UINT_MAX, format, ap);
  va_end(ap);
  return r;
}
