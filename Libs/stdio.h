#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned size_t;
#endif

typedef struct FILE FILE;

#define NULL ((void*)0)
#define EOF -1

#define stdin  ((FILE*)0) // Dummy
#define stdout ((FILE*)0) // Dummy

#define putc(c, stream) fputc(c, stream)

int putchar(int c);
int puts(const char *s);

// Not implemented:
int fclose(FILE *stream);
int fflush(FILE *stream);
int fgetc(FILE *stream);
FILE *fopen(const char *path, const char *mode);
int fputc(int c, FILE *stream);
int fputs(const char *s, FILE *stream);
int printf(const char *format, ...);
int sscanf(const char *s, const char *format, ...);
