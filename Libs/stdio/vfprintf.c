#include <limits.h>
#include <stdarg.h>
#include <stdio.h>

int vfprintf(FILE *stream, const char *format, va_list ap)
{
  return vsnprintf((char*)stream, UINT_MAX, format, ap);
}
