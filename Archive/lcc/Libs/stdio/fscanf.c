#include <stdarg.h>
#include <stdio.h>

int fscanf(FILE *stream, const char *format, ...)
{
  va_list ap;
  int r;
  va_start(ap, format);
  r = vfscanf(stream, format, ap);
  va_end(ap);
  return r;
}
