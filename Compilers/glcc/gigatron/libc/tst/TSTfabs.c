#include <stdio.h>
#include <math.h>


void test(double x, double y)
{
	printf("x=%f, y=%f, fabs(x)=%f, fabs(y)=%f, copysign(x,y)=%f\n",
	       x, y, fabs(x), fabs(y), copysign(x,y) );
}

int main()
{
	test(3.14,-1);
	test(-1, 3.14);
	test(2, 3.14);
	test(-12, -1);
	return 0;
}
