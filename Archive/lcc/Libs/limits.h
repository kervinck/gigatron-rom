
// Gigatron vCPU's plain characters are unsigned
#define CHAR_BIT 8
#define CHAR_MIN 0
#define CHAR_MAX 255
#define UCHAR_MAX 255
#define SCHAR_MIN -128
#define SCHAR_MAX  127

// 16-bit shorts
#define SHRT_MIN (-SHRT_MAX-1)
#define SHRT_MAX  32767
#define USHRT_MAX 65535U

// 16-bit integers
#define INT_MIN (-INT_MAX-1)
#define INT_MAX  32767
#define UINT_MAX 65535U

// 16-bit long (not ANSI compliant)
#define LONG_MIN (-LONG_MAX-1L)
#define LONG_MAX  32767L
#define ULONG_MAX 65535UL

// 16-bit long long (not ANSI compliant)
#define LLONG_MIN (-LLONG_MAX-1LL)
#define LLONG_MAX  32767LL
#define ULLONG_MAX 65535ULL

