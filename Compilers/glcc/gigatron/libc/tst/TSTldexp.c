#include <stdio.h>
#include <math.h>
#include <signal.h>


double pi = 3.141592539;

double x = 0.23456e18;
double y = 0.23456;

int main()
{
	int exp1;
	double p1, p2;
	p1 = frexp(pi, &exp1);
	p2 = ldexp(p1, exp1);
	printf("frexp(%.7e)=%.7e * 2^%d\n",pi, p1, exp1);
	printf("ldexp(%.7e,%d)=%.7e\n", p1, exp1, p2);

	p1 = frexp(x, &exp1);
	p2 = ldexp(p1, exp1);
	printf("frexp(%.7e)=%.7e * 2^%d\n",x, p1, exp1);
	printf("ldexp(%.7e,%d)=%.7e\n", p1, exp1, p2);

	p1 = frexp(0.0,&exp1);
	printf("frexp(0)=%.7e * 2^%d\n", p1, exp1);
	printf("ldexp(%.7e,%d)=%.7e\n", 0.0, -2, ldexp(0, -2));
	printf("ldexp(%.7e,%d)=%.7e\n", pi, -134, ldexp(pi, -134));
	printf("ldexp(%.7e,%d)=%.7e\n", pi, -200, ldexp(pi, -200));

	signal(SIGFPE,SIG_IGN);
	printf("ldexp(%.7e,%d)=%.7e\n", pi, +200, ldexp(pi, +200));
	signal(SIGFPE,SIG_DFL);
	printf("ldexp(%.7e,%d)=%.7e\n", pi, +200, ldexp(pi, +200));
	
	return 0;
}
