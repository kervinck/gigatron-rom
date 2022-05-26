#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

jmp_buf buf;

unsigned int fact(unsigned int n)
{
	unsigned int r,rr;
	if (n < 1)
		return 1;
	r = fact(n-1);
	rr = r * n;
	if (r != rr / n)
		longjmp(buf, 1); /* overflow */
	return rr;
}

unsigned long lfact(unsigned long n)
{
	unsigned long r,rr;
	if (n < 1)
		return 1;
	r = lfact(n-1);
	rr = r * n;
	if (r != rr / n)
		longjmp(buf, 1); /* overflow */
	return rr;
}

int main()
{
	register int i = 1234;
	register long j = 4321;
	register int r;

	r = setjmp(buf);
	if (! r) {
		for (i=0; i<100; i++) {
			printf("fact(%d)=", i);
			printf("%u\n", fact(i));
		}
	}
	if (r) {
		printf("overflow setjmp returns %d\n", r);
		printf("now i=%d and j=%ld\n", i, j);
	}
	r = setjmp(buf);
	if (! r) {
		for (j=0; j<100; j++) {
			printf("lfact(%ld)=", j);
			printf("%lu\n", lfact(j));
		}
	}
	if (r) {
		printf("overflow setjmp returns %d\n", r);
		printf("now i=%d and j=%ld\n", i, j);
	}
	return 0;
}
