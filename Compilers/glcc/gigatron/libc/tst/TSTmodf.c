#include <stdio.h>
#include <math.h>


#define NELEMS(x) (sizeof(x)/sizeof(x[0]))

double vals[] = { 1.5, 0.0, 1.0, -1.0, 2.0, -2.0,
		  234.25, -234.25, 1e19, 1e-23,
		  1000.2, 10000.2, 100000.2, 1000000.2, 10000000.2, -100000000.2 }; 


void test_modf(double x)
{
	double ip;
	double fp = modf(x, &ip);
	printf("modf(%.7g) -> %.7g + %.2g\n", x, ip, fp);
}

void test_floor(double x)
{
	printf("floor(%.7g)=%.7g, ceil(%.7g)=%.7g\n", x, floor(x), x, ceil(x));
}


int main()
{
	int i;
	printf("------------ modf\n");
	for (i=0; i<NELEMS(vals); i++)
		test_modf(vals[i]);
	printf("------------ floor/ceil\n");
	for (i=0; i<NELEMS(vals); i++)
		test_floor(vals[i]);
	return 0;
}
