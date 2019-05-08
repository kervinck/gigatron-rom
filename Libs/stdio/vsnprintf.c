
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

// TODO Handle EOF error from putc
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
 *  char pointers into memory. Streams are special pointer values
 *  representing file descriptors, and not true FILE* pointers.
 *  XXX For now, we even only recognize NULL/0 for stdout and/or stdin.
 */
#define _isstream(str) ((int)(str) == 0)

/*----------------------------------------------------------------------+
 |      Data                                                            |
 +----------------------------------------------------------------------*/

static const unsigned radixtable[] = { 1, 10, 100, 1000, 10000 };

/*----------------------------------------------------------------------+
 |      Functions                                                       |
 +----------------------------------------------------------------------*/

// Helpers
static const char *_parsenum(const char *str, int *num);
static char *_charout(int c, char *str);

/*----------------------------------------------------------------------+
 |       vsnprintf                                                      |
 +----------------------------------------------------------------------*/

int vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
  #define charout(c) (str = _charout((c), str))
  enum { minus = 1, plus = 2, zero = 4, space = 8, hash = 16 };

  int count = 0; // Return value is the number of chars produced
  int flags, width, precision;

  // For conversion
  int sign, r, digit, m, radix;
  const char *s;

  for (; *format; format++) {
    if (*format != '%') {
      charout(*format);         // Verbatim
      count++;
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
        count++;
        sign = 0;
        digit += '0';           // digit != 0 causes padding with zeroes
      }

      // Emit digits
      for (r=4; r>=0; r--) {
        radix = radixtable[r];  // Simpler and faster than division/modulo
        while (1) {
          m -= radix;
          if (m < 0)            // This relies on signed overflow for %u
            break;
          digit++;
        }
        m += radix;

        if (digit || radix == 1U) {// Non-zero or last digit?
          while (--width > r) { // Padding with spaces or zeroes
            charout(flags & zero ? '0' : ' ');
            count++;
          }
          if (sign) {
            charout(sign);      // Emit sign before first digit
            count++;
            sign = 0;
          }
          charout(digit|'0');   // Emit digit as printable ASCII
          count++;
          digit = '0';          // Enable all following digits
        }
      }
      break;
    case 'c':                   // int: single character, cast to unsigned char
      charout(va_arg(ap, unsigned char));
      count++;
      break;
    case 's':                   // char*s: string
      for (s=va_arg(ap, const char*); *s; s++) {
        charout(*s);
        count++;
      }
      break;
    case 'n':                   // int*n: count write-back, no conversion
      *va_arg(ap, int*) = count;
      break;
    case '%':                   // literal %-sign, no conversion
      charout('%');
      count++;
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

  if (!_isstream(str))
    *str = '\0';                // Terminating zero for strings

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

static char *_charout(int c, char *str)
{
  if (_isstream(str))
    putc(c, (FILE*)str);        // To stream
  else
    *str++ = c;                 // To string
  return str;
}

/*----------------------------------------------------------------------+
 |                                                                      |
 +----------------------------------------------------------------------*/

