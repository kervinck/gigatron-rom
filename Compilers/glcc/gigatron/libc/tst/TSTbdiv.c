#include <stdlib.h>
#include <stdio.h>

void checkdiv(int a, int q)
{
	div_t r = div(a, q);
	printf("div(%d, %d) = {%d, %d}\n", a, q, r.quot, r.rem);
}

void checkldiv(long a, long q)
{
	ldiv_t r = ldiv(a, q);
	printf("ldiv(%ld, %ld) = {%ld, %ld}\n", a, q, r.quot, r.rem);
}

int main()
{
	checkdiv(12345,123);
	checkdiv(12345,-123);
	checkdiv(-12345,123);
	checkldiv(12345,123);
	checkldiv(12345,-123);
	checkldiv(-12345,123);
	return 0;
}
