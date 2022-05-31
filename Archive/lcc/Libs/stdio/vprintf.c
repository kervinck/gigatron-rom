#include <limits.h>
#include <stdio.h>

int vprintf(const char *format, va_list ap)
{
  return vsnprintf((char*)stdout, UINT_MAX, format, ap);
}
