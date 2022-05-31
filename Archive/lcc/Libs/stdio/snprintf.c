#include <stdarg.h>
#include <stdio.h>

int snprintf(char *str, size_t size, const char *format, ...)
{
  int r;
  va_list ap;
  va_start(ap, format);
  r = vsnprintf(str, size, format, ap);
  va_end(ap);
  return r;
}
