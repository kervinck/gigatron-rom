
/*----------------------------------------------------------------------+
 |                                                                      |
 |      vsnprintf.c -- the heart of the fprintf/sprintf family          |
 |                                                                      |
 +----------------------------------------------------------------------*/

/*
 *  Copyright (C) 2019, Marcel van Kervinck
 *  All rights reserved
 *
 *  Please read the enclosed file `LICENSE' or retrieve this document
 *  from https://github.com/kervinck/gigatron-rom for terms and conditions.
 */

// TODO Handle size, flags, width and precision
// TODO Implement %o %x %X %p
// TODO Cross-check with C89 standard instead of K&R2

/*----------------------------------------------------------------------+
 |      Includes                                                        |
 +----------------------------------------------------------------------*/

// C standard
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>

/*----------------------------------------------------------------------+
 |      Definitions                                                     |
 +----------------------------------------------------------------------*/

/*
 *  Our vsnprintf can work with string outputs and streams. This way
 *  we don't need to duplicate the implementation for the formatting
 *  logic, or work with intermediate buffers in memory. Strings are
 *  char pointers into memory. Streams are passed through the "backdoor"
 *  as `size' and marked with a NULL value for `str'.
 */

/*----------------------------------------------------------------------+
 |      Data                                                            |
 +----------------------------------------------------------------------*/

static const unsigned radixtable[] = { 1, 10, 100, 1000, 10000 };

/*----------------------------------------------------------------------+
 |      Functions                                                       |
 +----------------------------------------------------------------------*/

// Helpers
static const char *_parsenum(const char *str, int *num);
static int _discard(int c, FILE *stream);

/*----------------------------------------------------------------------+
 |       vsnprintf                                                      |
 +----------------------------------------------------------------------*/

int vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
  enum { minus = 1, plus = 2, zero = 4, space = 8, hash = 16 };

  int count = 0; // Return value is the number of chars produced
  int flags, width, precision;

  // For conversion
  int sign, i, digit, m, radix;
  const char *s;

  struct _iobuf strbuf, *stream;

  // Write out strings through a dummy FILE structure instead of str
  #define charout(c) do{\
    if (putc(c, stream) < 0)\
      return EOF;\
    count++;\
  }while(0)

  // String or stream?
  if (str) {                    // String
    if (size == 0)
      return 0;                 // No space for anything
    strbuf._ptr = str;
    strbuf._avail = size;
    strbuf._flush = _discard;
    stream = &strbuf;
  } else                        // Stream
    stream = (FILE*)size;

  for (; *format; format++) {
    if (*format != '%') {
      charout(*format);         // Verbatim
      continue;
    }
    // Process formatter in the remainder of body

    // Parse flags
    flags = 0;
    while (1) {
      switch (*++format) {
      case '-': flags |= minus; continue;
      case '+': flags |= plus;  continue;
      case '0': flags |= zero;  continue;
      case ' ': flags |= space; continue;
      case '#': flags |= hash;  continue;
      }
      break;
    }

    // Parse minimum width
    width = -1;
    if (*format == '*') {
      width = va_arg(ap, int);
      format++;
    } else
      format = _parsenum(format, &width);

    // Parse floating point precision or maximum string width
    precision = -1;
    if (*format == '.') {
      format++;
      if (*format == '*') {
        precision = va_arg(ap, int);
        format++;
      } else
        format = _parsenum(format, &precision);
    }

    // Parse length modifier
    switch (*format) {
    case 'h': // short or unsigned short
    case 'l': // long or unsigned long
    case 'L': // long double
      format++;
      break;
    }

    // Process conversion
    switch (*format) {
    case 'd': case 'i':         // int: signed decimal
    case 'u':                   // int: unsigned decimal
      m = va_arg(ap, int);

      // Split magnitude from sign
      digit = sign = 0;
      if (m < 0) {
        if (*format == 'u') {   // Large unsigned int
          m -= 30000;           // This can have signed overflow
          digit = 3;            // Make first digit safe for radix loop below
        } else {                // Negative signed int
          m = -m;               // Don't worry about -INT_MIN overflow
          sign = '-';
          width--;
        }
      }

      if (!sign && (flags & plus)) {
        sign = '+';             // Force plus sign
        width--;
      }

      if (sign && (flags & zero)) {
        charout(sign);          // Emit sign first when padding with zeroes
        sign = 0;
        digit += '0';           // digit != 0 causes padding with zeroes
      }

      // Emit digits
      for (i=4; i>=0; i--) {
        radix = radixtable[i];  // Simpler and faster than division/modulo
        while (1) {
          m -= radix;
          if (m < 0)            // This relies on signed overflow for %u
            break;
          digit++;
        }
        m += radix;

        if (digit || radix == 1U) {// Non-zero or last digit?
          while (--width > i)   // Padding with spaces or zeroes
            charout(flags & zero ? '0' : ' ');
          if (sign) {
            charout(sign);      // Emit sign before first digit
            sign = 0;
          }
          charout(digit|'0');   // Emit digit as printable ASCII
          digit = '0';          // Enable all following digits
        }
      }
      break;
    case 'c':                   // int: single character, cast to unsigned char
      charout(va_arg(ap, unsigned char));
      break;
    case 's':                   // char*s: string
      for (s=va_arg(ap, const char*); *s; s++)
        charout(*s);
      break;
    case 'n':                   // int*n: count write-back, no conversion
      *va_arg(ap, int*) = count;
      break;
    case '%':                   // literal %-sign, no conversion
      charout('%');
      break;
#if 0
    case 'o':                   // int: unsigned octal (without a leading zero)
    case 'x': case 'X':         // int: unsigned hex (without leading 0x or 0X)
    case 'p':                   // void*: implementation-specific pointer
    case 'f':                   // double: [-]mmm.ddd, default precision 6, no decimal point for 0
    case 'e': case 'E':         // double: [-]m.dddddde+/-xx or [-]m.ddddddE+/-xx
    case 'g': case 'G':         // double: %e/%E if xx<-4 or xx >= precision, otherwise %f
#endif
    default:                    // Unknown or not implemented conversion
      charout('@');
      (void) va_arg(ap, int);   // Swallow an argument and hope for the best
      break;
    }
  }

  if (str)
    *stream->_ptr = '\0';       // Terminating zero for strings

  return count;
}

/*----------------------------------------------------------------------+
 |      Helpers                                                         |
 +----------------------------------------------------------------------*/

/*
 *  Parse optional number from string, returns advanced pointer.
 *  Only writes back the result through *num in case a number is present.
 */
static const char *_parsenum(const char *s, int *num)
{
  int n = 0;
  while (isdigit(*s)) {
    n = n * 10 + *s++ - '0';
    *num = n;
  }
  return s;
}

/*
 *  Throw away chars that don't fit in the caller's string space
 */
static int _discard(int c, FILE *stream)
{
  return (stream->_avail = 1);
}

/*----------------------------------------------------------------------+
 |                                                                      |
 +----------------------------------------------------------------------*/

