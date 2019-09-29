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
  size_t _n;                    // Range is 0..sizeof(buffer) for read,
                                // and 1...sizeof(buffer)+1 for write.
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

extern struct _stdin _stdin;
extern struct _stdout _stdout;
#define stdin ((FILE*)&_stdin)
#define stdout ((FILE*)&_stdout)
#define stderr stdout

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

#define getc(stream) fgetc(stream)

int getchar(void);
int fgetc(FILE *stream);
int ungetc(int c, FILE *stream);
int scanf(const char *format, ...);
int fscanf(FILE *stream, const char *format, ...);
int vfscanf(FILE *stream, const char *format, va_list arg);

// Not yet implemented:
char *gets(char *str);
char *fgets(char *str, int size, FILE *stream);
int sscanf(const char *s, const char *format, ...);
