#include <limits.h>
#include <stdio.h>

int vsprintf(char *str, const char *format, va_list ap)
{
  return vsnprintf(str, UINT_MAX, format, ap);
}
