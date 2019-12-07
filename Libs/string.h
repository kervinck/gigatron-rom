#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned size_t;
#endif

char *strcpy(char *dst, const char *src);
void *memcpy(void *dst, const void *src, size_t n);
void *memset(void *b, int c, size_t len);

// Not implemented:
char *strchr(const char *s, int c);
int strcmp(const char *s1, const char *s2);
char *strerror(int errnum);
int strncmp(const char *s1, const char *s2, size_t n);
