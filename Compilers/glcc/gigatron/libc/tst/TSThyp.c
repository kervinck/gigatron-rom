#include <stdio.h>
#include <math.h>

void go(double x)
{
	double s = sinh(x);
	double c = cosh(x);
	double t = tanh(x);
	printf("x=%6.2f\tsinh=%12.8g cosh=%12.8g tanh=%12.8g\n", x, s, c, t);
}

int main()
{
	int i;
	for (i=-50; i<=50; i++)
		go(i*1.7/5.0);
	return 0;
}
