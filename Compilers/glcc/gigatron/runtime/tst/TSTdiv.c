#include <stdio.h>

#ifndef __gigatron__
# define int short
#endif

int rand()
{
	static int whatever = 0x212;
	whatever = whatever * 6969 + 0x1234;
	return whatever;
}


void chk(int i, int j)
{
	unsigned int ui = i;
	unsigned int uj = j;
	if (j != 0) {
		printf("divu: %u/%u = %u\t", ui, uj, (unsigned int)(ui/uj));
		printf("divs: %d/%d = %d\n", i, j, (int)(i/j));
		printf("modu: %u%%%u = %u\t", ui, uj, (unsigned int)(ui%uj));
		printf("mods: %d%%%d = %d\n", i, j, (int)(i%j));
	}
}


int main()
{
	int i;
	chk(1,1);
	chk(-32768,234);
	chk(234,-32768);
	chk(-32768,-32768);
	for (i=0; i<200; i++) {
		int a = rand();
		int b = rand();
		int c = rand() & 0x7;
		b = b >> c;
		chk(a,b);
		rand();
	}
	return 0;
}
