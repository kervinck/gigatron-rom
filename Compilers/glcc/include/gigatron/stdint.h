#ifndef __STDINT
#define __STDINT

/* int64_t and friends do not exist on the Gigatron */

typedef signed char     int8_t;
typedef signed short    int16_t;
typedef signed long     int32_t;

typedef unsigned char   uint8_t;
typedef unsigned short  uint16_t;
typedef unsigned long   uint32_t;

typedef int8_t    int_least8_t;
typedef int16_t   int_least16_t;
typedef int32_t   int_least32_t;

typedef uint8_t   uint_least8_t;
typedef uint16_t  uint_least16_t;
typedef uint32_t  uint_least32_t;

typedef int8_t    int_fast8_t;
typedef int16_t   int_fast16_t;
typedef int32_t   int_fast32_t;

typedef uint8_t   uint_fast8_t;
typedef uint16_t  uint_fast16_t;
typedef uint32_t  uint_fast32_t;

typedef int16_t   intptr_t;
typedef uint16_t  uintptr_t;
typedef int32_t   intmax_t;
typedef uint32_t  uintmax_t;

#define INT8_C(v)    (v)
#define INT16_C(v)   (v)
#define INT32_C(v)   (v ## L)
#define INTMAX_C(v)  INT32_C(v)

#define UINT8_C(v)   (v)
#define UINT16_C(v)  (v)
#define UINT32_C(v)  (v ## UL)
#define UINTMAX_C(v) UINT32_C(v)

#define INT8_MAX         127
#define INT16_MAX        32767
#define INT32_MAX        2147483647
#define INT8_MIN         -128
#define INT16_MIN        -32768
#define INT32_MIN        -2147483648
#define UINT8_MAX         255
#define UINT16_MAX        65535
#define UINT32_MAX        4294967295U

#define INT_LEAST8_MIN    INT8_MIN
#define INT_LEAST16_MIN   INT16_MIN
#define INT_LEAST32_MIN   INT32_MIN
#define INT_LEAST8_MAX    INT8_MAX
#define INT_LEAST16_MAX   INT16_MAX
#define INT_LEAST32_MAX   INT32_MAX
#define UINT_LEAST8_MAX   UINT8_MAX
#define UINT_LEAST16_MAX  UINT16_MAX
#define UINT_LEAST32_MAX  UINT32_MAX

#define INT_FAST8_MIN     INT8_MIN
#define INT_FAST16_MIN    INT16_MIN
#define INT_FAST32_MIN    INT32_MIN
#define INT_FAST8_MAX     INT8_MAX
#define INT_FAST16_MAX    INT16_MAX
#define INT_FAST32_MAX    INT32_MAX
#define UINT_FAST8_MAX    UINT8_MAX
#define UINT_FAST16_MAX   UINT16_MAX
#define UINT_FAST32_MAX   UINT32_MAX

#define INTPTR_MAX        INT16_MAX
#define INTPTR_MIN        INT16_MIN
#define UINTPTR_MAX       UINT16_MAX

#define INTMAX_MAX        INT32_MAX
#define INTMAX_MIN        INT32_MIN
#define UINTMAX_MAX       UINT32_MAX

#define PTRDIFF_MIN       INTPTR_MIN
#define PTRDIFF_MAX       INTPTR_MAX
#define SIZE_MAX          UINTPTR_MAX

#define WCHAR_MAX         UINT16_MAX
#define WCHAR_MIN         0
#define WINT_MAX          INT16_MAX
#define WINT_MIN          INT16_MIN

#endif
