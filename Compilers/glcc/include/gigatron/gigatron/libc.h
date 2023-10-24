#ifndef __GIGATRON_LIBC
#define __GIGATRON_LIBC

#include <stdlib.h>
#include <stdio.h>

/* ==== Nonstandard functions defined in libc ==== */


/* ---- Program startup and exit ---- */

/* Exit the program without running the finalization functions like exit().
   This just calls _exitm() without message. */
extern void _exit(int retcode);

/* Exit with a return code_and an optional message.
   The libc version of _exitm does the following:
   - calls the function pointer `_exitm_msgfunc` if nonzero
   - flashes a pixel on the first screen line at a position
     indicative of the return code. */
extern void _exitm(int ret, const char *msg);
extern void (*_exitm_msgfunc)(int ret, const char *msg);

/* Arrange for initialization function func() to be called before main(). 
   Only one of those can exist per c file. */
#define DECLARE_INIT_FUNCTION(func) \
   static struct { void(*f)(void); void *next; \
   } __glink_magic_init = { func, 0 }

/* Arrange for finalization function func() to be called when main() returns. 
   Only one of these can exist per c file. */
#define DECLARE_FINI_FUNCTION(func) \
   static struct { void(*f)(void); void *next; \
   } __glink_magic_fini = { func, 0 }


/* ----- Raising signals ----- */

/* Raising signal with an error message */
extern void _raisem(int signo, const char *msg);

/* The following vector changes what raise() does
   without requiring all the context saving that
   is necessary for running a full signal handler. */
extern void *_raise_disposition;

/* Legal values for raise disposition */
#define RAISE_EXITS ((void*)0)
#define RAISE_EMITS_SIGNAL ((void*)&_raise_emits_signal)
#define RAISE_SETS_CODE ((void*)&_raise_sets_code)

/* Support for the above. Do not call. */
extern const char _raise_emits_signal;
extern const char _raise_sets_code;

/* Setting _raise_disposition to RAISE_SETS_CODE
   simply writes the signal code into this variable.
   Low byte is the signal code, high byte is the fp code. */
extern int _raise_code;


/* ---- Numerics ---- */

/* Elementary long constants */
extern double _lzero, _lone, _ltwo, _lminus;

/* Elementary floating point constants */
extern double _fzero, _fone, _fhalf, _ftwo, _fminus;

/* Value of PI assumed correct by the trigonometric functions! */
extern double _pi, _pi_over_2, _pi_over_4;

/* Raise a SIGFPE exception and return defval if the exception is ignored.
   If a signal handler for SIGFPE has been setup, these functions
   return what the signal handler returns. */
extern double _fexception(double defval);
extern double _foverflow(double defval);

/* Multiplies x by 10^n. */
extern double _ldexp10(double x, int n);

/* Returns a double y and an exponent exp such that x = y * 10^exp,
   with y as large as possible while keeping an exact integer part. */
extern double _frexp10(double x, int *pexp);

/* Like the C99 function remquo but with fmod-style remainder. */
extern double _fmodquo(double x, double y, int *quo);

/* Evaluate polynomials */
extern double _polevl(double x, double *coeff, int n);
extern double _p1evl(double x, double *coeff, int n);


/* ---- Stdio ---- */

/* Low-level open function called by fopen/freopen. The default
   version always returns -1 and sets errno to ENOTSUP.  Should be
   called with fp->_flag containing the desired read/write mode
   and fp->_file set to -1 or set to a desired file descriptor. */
extern int _openf(FILE *fp, const char *fname);

/* ---- Bitsets ---- */

extern void _bitset_clear(char *set, size_t sz);
extern void _bitset_compl(char *set, size_t sz);
extern void _bitset_set(char *set, unsigned int i);
extern void _bitset_clr(char *set, unsigned int i);
extern int  _bitset_test(char *set, unsigned int i);


/* ---- Converting numbers to strings ---- */

/* Using these functions help avoiding the bulky printf */

/* Functions to convert integers to strings. The buffer should be
   eight bytes long for ints and sixteen bytes for longs. Radix should
   be in range 8 to 36. Smaller radix might overflow the buffer.  Note
   that the return value is not usually equal to buffer because the
   digits are generated backwards and stored from the end of the
   buffer marching towards the beginning. */

extern char *itoa(int value, char *buf8, int radix);
extern char *utoa(unsigned int value, char *buf8, int radix);
extern char *ltoa(long value, char *buf16, int radix);
extern char *ultoa(unsigned long value, char *buf16, int radix);

/* Function to convert doubles to strings using the 'e', 'f', or 'g'
   style of the printf function. The buffer should be large enough for
   the required precision and number. Using the 'f' style with 
   large numbers can generate long strings. */

extern char *dtoa(double x, char *buf, char format, int prec);


/* ---- Misc ---- */

/* Calls srand(int) using the gigatron entropy generator.
   Using srand(0) has the same effect. */
extern void _srand(void);

/* Scans memory region [s,s+n) and return a pointer to the first byte 
   equal to either c0 or c1. Return zero if not found. 
   This is fast when there is a SYS call. */
extern void *_memchr2(const void *s, char c0, char c1, size_t n);
extern void *__memchr2(const void *s, int c0c1, size_t n);

/* Scans memory region [s,s+n) in bank given by bits 6 and 7 of bank.
   Return a pointer to the first byte equal to either c0 or c1.
   Return zero if not found. This is fast when there is a SYS call. */
extern void *_memchr2ext(char bank, const void *s, char c0, char c1, size_t n);
extern void *__memchr2ext(char bank, const void *s, int c0c1, size_t n);

/* Copy a block of memory [src,src+n) to [dst,dst+n) across memory banks.
   The destination bank is given by bits 6 and 7 of argument banks,
   and the source bank is given by bits 5 and 4.
   Returns zero when no expansion. */
extern void *_memcpyext(char banks, void *dst, const void* src, size_t n);

/* Same as strtok but uses the last argument as context */
extern char *_strtok(char *str, const char *delim, char **ps);

/* Swap two memory blocks of size n. */
extern void _memswp(void *a, void *b, size_t n);


/* ---- Clock ---- */

/* The standard function clock() returns a clock_t which is a long
   but only contains 24 bits numbers (0 to 16M). The alternative
   entry point _clock() returns a 16 bits integer which is often
   sufficient and avoids long int overhead. */
extern unsigned int _clock(void);

/* Wait for n vertical blanks (or n clocks) */
extern void _wait(int n);

#endif
