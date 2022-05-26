#include <stdio.h>

#define NELEMS(a) (sizeof(a)/sizeof(a[0]))

#ifdef __gigatron__
# define dword long
#else
# define dword int
#endif


dword p[] = { 0, 1, 16781313, 276828288,
	      255, 65535, 16777215, -1,
	      305419896, -1412568388, 178301450, 1442144086 };
	
	
int main()
{
	int i, j;
	unsigned dword *q = (unsigned dword *)p;
	
	for (i=0; i<NELEMS(p); i++) {
		printf("\n");
		for (j=0; j<32; j++)
			printf("0x%lx << %d = 0x%lx\n", (unsigned long)q[i], j, (unsigned long)(unsigned dword)(q[i]<<j));
	}
	return 0;
}
