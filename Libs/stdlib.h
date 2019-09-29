#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned size_t;
#endif

#define EXIT_SUCCESS 0
#define EXIT_FAILURE -1

// Not implemented:
void exit(int status);
void qsort(void *base, size_t nel, size_t width, int (*compar)(const void *, const void *));
