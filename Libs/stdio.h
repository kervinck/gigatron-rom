#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned size_t;
#endif

#ifndef _VA_LIST
#define _VA_LIST
typedef char *va_list;
#endif

typedef struct FILE FILE;

#define NULL ((void*)0)
#define EOF -1

#define stdin  ((FILE*)0) // Dummy
#define stdout ((FILE*)0) // Dummy

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

// Not implemented:
int fclose(FILE *stream);
int fflush(FILE *stream);
int fgetc(FILE *stream);
FILE *fopen(const char *path, const char *mode);
int sscanf(const char *s, const char *format, ...);
