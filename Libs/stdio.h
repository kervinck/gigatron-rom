#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned size_t;
#endif

#ifndef _VA_LIST
#define _VA_LIST
typedef char *va_list;
#endif

typedef struct _iobuf FILE;

struct _iobuf {
  char *_ptr;                   // Current pointer in buffer
  size_t _avail;                // Valid range is 1...sizeof(buffer)+1
  int (*_flush)(int,FILE*);     // Same prototype as fputc()
  char _flags;
};

// https://minnie.tuhs.org/cgi-bin/utree.pl?file=V7/usr/include/stdio.h
enum {
  _ioread  = 1,
  _iowrite = 2,
  _ioeof   = 16,
  _ioerr   = 32,
};

#define NULL ((void*)0)
#define EOF -1

extern struct _stdin _stdin; // Not yet implemented
extern struct _stdout _stdout;
extern struct _stderr _stderr; // Not yet implemented
#define stdin ((FILE*)&_stdin)
#define stdout ((FILE*)&_stdout)
#define stderr ((FILE*)&_stderr)

#define putc(c, stream) fputc(c, stream)

int putchar(int c);
int puts(const char *s);
int fputc(int c, FILE *stream);
int printf(const char *format, ...);
int fputs(const char *s, FILE *stream);
int vprintf(const char *format, va_list ap);
int sprintf(char *str, const char *format, ...);
int fprintf(FILE *stream, const char *format, ...);
int vsprintf(char *str, const char *format, va_list ap);
int vfprintf(FILE *stream, const char *format, va_list ap);
int snprintf(char *str, size_t size, const char *format, ...);
int vsnprintf(char *str, size_t size, const char *format, va_list ap);

FILE *fopen(const char *path, const char *mode);
int fflush(FILE *stream);
int fclose(FILE *stream);

// Not implemented:
int fgetc(FILE *stream);
int sscanf(const char *s, const char *format, ...);
