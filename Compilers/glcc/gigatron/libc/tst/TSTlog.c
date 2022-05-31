#include <math.h>
#include <stdio.h>
#include <signal.h>


#ifdef __gigatron__
double sigfpe(int signo, int fpeinfo)
{
	printf("[SIGFPE %d %d] ", signo, fpeinfo);
	signal(SIGFPE, (sig_handler_t)sigfpe);
	return 123456789.0;
}
#endif

void testexp(double x)
{
	printf("exp(%.8g)=%.8g\n", x, exp(x));
}

void testlog(double x)
{
	double y = x/10;
	printf("log(%.8g)=%.8g\n", x, log(x));
	printf("log(1+%.8g)=%.8g\n", y, log(1+y)); 
}

int main()
{
	int i;

#ifdef __gigatron__
	signal(SIGFPE, (sig_handler_t)sigfpe);
#endif

	for (i=1; i<10; i++) {
		testexp((double)i);
		testexp((double)i+0.5);
	}
	testexp(1e-4);
	testexp(100);

	for (i=1; i<10; i++) {
		testlog((double)i);
		testlog((double)i+0.5);
	}
	testlog(1e23);
	testlog(0);
	testlog(-1);
	
	
	return 0;
}
