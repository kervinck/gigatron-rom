#include <stdio.h>

#ifdef __gigatron__
# define dword long
#else
# define dword int
#endif


dword rand()
{
	static dword whatever = 0;
	whatever = whatever * (dword)1664525L + (dword)1013904223L;
	return whatever;
}


void chk(dword i, dword j)
{
	unsigned dword ui = i;
	unsigned dword uj = j;
	if (j != 0) {
		printf("divu: %lu / %lu = %lu\t", (unsigned long)ui, (unsigned long)uj, (unsigned long)(ui/uj));
		printf("divs: %ld / %ld = %ld\n", (long)i, (long)j, (long)(i/j));
		printf("modu: %lu %% %lu = %lu\t", (unsigned long)ui, (unsigned long)uj, (unsigned long)(ui%uj));
		printf("mods: %ld %% %ld = %ld\n", (long)i, (long)j, (long)(i%j));
	}
}


int main()
{
	int i;

	chk(1,1);
	chk((dword)0x80000000UL,234);
	chk(234,(dword)0x80000000UL);
	chk((dword)0x80000000UL,(dword)0x80000000UL);
	for (i=0; i<200; i++) {
		dword a = rand();
		dword b = rand();
		dword c = rand() & 0x7;
		b = b >> c;
		chk(a,b);
		rand();
	}
	return 0;
}
