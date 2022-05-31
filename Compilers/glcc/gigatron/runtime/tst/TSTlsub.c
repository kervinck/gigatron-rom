#include <stdio.h>

#define NELEMS(a) (sizeof(a)/sizeof(a[0]))

#ifdef __gigatron__
# define dword unsigned long
#else
# define dword unsigned int
#endif

typedef unsigned long ul;

dword p[] = { 0x00000000LU, 0x00000001LU, 0x01001001LU, 0x10801080LU,
	      0x000000ffLU, 0x0000ffffLU, 0x00ffffffLU, 0xffffffffLU,
	      0x12345678LU, 0xabcdeabcLU, 0x0aa0aa0aLU, 0xaa0aa0aaLU };
	
	
int main()
{
	int i, j;
	
	for (i=0; i<NELEMS(p); i++) {
		printf("\n");
		for (j=0; j<NELEMS(p); j++) {
			printf("%08lx - %08lx = %08lx\t", (ul)p[i], (ul)p[j], (ul)(p[i] - p[j]));
			printf("%08lx - %08lx = %08lx\t", (ul)p[i], (ul)j, (ul)(p[i] - (dword)j));
			printf("%08lx- %08lx = %08lx\n", (ul)(dword)(long)(-i), (ul)p[j], (ul)((dword)(long)(-i) - p[j]));
		}
		printf("- %08lx = %08lx\n", (ul)p[i], (ul)(dword)(-(long)(p[i])));
	}
	return 0;
}
