#ifndef __GIGATRON_PRINTF
#define __GIGATRON_PRINTF

#if !defined(_SIZE_T) && !defined(_SIZE_T_) && !defined(_SIZE_T_DEFINED)
#define _SIZE_T
#define _SIZE_T_
#define _SIZE_T_DEFINED
typedef unsigned int size_t;
#endif

#if !defined(_VA_LIST) && !defined(_VA_LIST_DEFINED)
#define _VA_LIST
#define _VA_LIST_DEFINED
typedef char *__va_list;
#endif

/* ========================================
   PRINTF-LIKE FUNCTIONS FROM STDIO.H
   ======================================== */

/* Must exactly match the stdio declarations */

extern int printf(const char *, ...);
extern int sprintf(char *, const char *, ...);
extern int vprintf(const char *, __va_list);
extern int vsprintf(char *, const char *, __va_list);

/* extern int fprintf(FILE *, const char *, ...); */
/* extern int vfprintf(FILE *, const char *, __va_list); */


/* ========================================
   PRINTF-LIKE FUNCTIONS FOR THE CONSOLE 
   ======================================== 

   These functions are similar to the stdio printf functions but they
   bypass stdio and hit directly the console. The cprintf function
   still import the relatively heavy printf machinery but not the
   stdio machinery. The midcprintf and mincprintf functions are
   considerably smaller but with limited capabilities. */


/* Print formatted text at the cursor position */
extern int cprintf(const char *fmt, ...);

/* Print formatted text like cprintf except that it is called
   with a va_list instead of a variable number of arguments. */
extern int vcprintf(const char *fmt, __va_list ap);

/* Alternate cprintf functions with less capabilities.
   Function mincprintf only understands %d and %s without
   qualifications.  Function midcprintf also understands %c, %i, %u,
   %o, %x with numeric field sizes.  None of these functions handles
   longs or floating point numbers. */
extern int mincprintf(const char *fmt, ...);
extern int midcprintf(const char *fmt, ...);



/* ========================================
   PRINTF CAPABILITY SELECTION
   ======================================== */

/* Function __doprint() is the formatting routine underlying all the
   printf functions except mincprintf() and midcprintf(). It is
   aliased to either _doprint_c89() or _doprint_simple() depending on
   the linker option
      --option=PRINTF_C89, or
      --option=PRINTF_SIMPLE.
   The same can be achieved with, for instance,
      #pragma glcc option("PRINTF_SIMPLE")
   The default is _doprint_c89(). */
extern int _doprint(const char*, __va_list);

/* The _doprint_c89 formatting routine complies with the ANSI C89
   specification which is unfortunately complex. This formatting
   functions requires 2KB bytes of code, plus additional code
   to support longs and doubles which is only linked if longs
   or doubles are used elsewhere in the program. */
extern int  _doprint_c89(const char*, __va_list);

/* The _doprint_simple formatting routine only provides support for
   conversion characters %c, %s, %d, %i, %u, %x, %o with optional
   field length. No attempt is made to support longs and doubles.
   This formatting functions requires about 750 bytes of code.
   The same code is used directly by function midcprinf. */
extern int  _doprint_simple(const char*, __va_list);


#endif
