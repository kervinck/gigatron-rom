#include <limits.h>
#include <stdarg.h>
#include <stdio.h>

int printf(const char *format, ...)
{
  int r;
  va_list ap;
  va_start(ap, format);
  r = vsnprintf(NULL, (size_t)stdout, format, ap);
  va_end(ap);
  if (r >= 0 && fflush(stdout) != 0)
    return EOF;
  return r;
}
