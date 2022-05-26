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
		for (j=0; j<NELEMS(p); j++) {
			printf("%ld <= %ld = %c\t", (long)p[i], (long)p[j], (p[i]<=p[j])?'T':'F');
			printf("%ld < %ld = %c\t", (long)p[i], (long)p[j], (p[i]<p[j])?'T':'F');
			printf("%lu > %lu = %c\t", (long)q[i], (long)q[j], (q[i]>q[j])?'T':'F');
			printf("%ld == %ld = %c\n", (long)p[i], (long)p[j], (p[i]==p[j])?'T':'F');
		}
	}
	return 0;
}
