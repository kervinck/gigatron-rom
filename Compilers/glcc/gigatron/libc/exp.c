#include <gigatron/libc.h>
#include <errno.h>
#include <math.h>


/* Using the Cephes method which is an overkill. */

static double P[] = {
		     1.26177193074810590878E-4,
		     3.02994407707441961300E-2,
		     9.99999999999999999910E-1,
};

static double Q[] = {
		     3.00198505138664455042E-6,
		     2.52448340349684104192E-3,
		     2.27265548208155028766E-1,
		     2.00000000000000000009E0,
};
static double C1 = 6.93145751953125E-1;
static double C2 = 1.42860682030941723212E-6;

static double LOG2E = 1.4426950408889634073599;   /* 1/log(2) */

double exp(register double x)
{
	double xx;
	double px = floor( LOG2E * x + 0.5 );
	register int n = px;
	if (px >= 256) {
		return _foverflow(HUGE_VAL);
	}
	if (px < -256)
		return 0;
	x -= px * C1;
	x -= px * C2;
	xx = x * x;
	px = x * _polevl(xx, P, 2);
	x = px / (_polevl(xx, Q, 3) - px);
	return ldexp(_fone + _ftwo * x, n);
}
