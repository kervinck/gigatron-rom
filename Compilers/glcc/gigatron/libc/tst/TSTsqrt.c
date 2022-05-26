#include <stdio.h>
#include <math.h>
#include <signal.h>


void test_sqrt(double x)
{
	double y;
	y = sqrt(x);
	printf("sqrt(%.8g) = %.8g (squared %.8g)\n", x, y, y*y);
}

#define NELEMS(x) (sizeof(x)/sizeof(x[0]))

double vals[] = { 1.5, 0.0, 1.0, -1.0, 2.0, -2.0,
		  234.25, -234.25, 1e19, 1e-23,
		  1000.2, 10000.2, 100000.2, 1000000.2, 10000000.2, -100000000.2 }; 

double sigfpe(int signo, int fpeinfo)
{
	printf("[SIGFPE %d %d] ", signo, fpeinfo);
	signal(SIGFPE, (sig_handler_t)sigfpe);
	return 123456789.0;
}


int main()
{
	int i=0;

#ifdef __gigatron__
	signal(SIGFPE, (sig_handler_t)sigfpe);
#endif
	for (i=2; i<50; i+=3)
		test_sqrt((double)i);
	for (i=0; i<NELEMS(vals); i++)
		test_sqrt(vals[i]);
	return 0;
}
