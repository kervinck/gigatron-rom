#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <gigatron/libc.h>

/* From Cephes:
   arctan(x)  = x + x^3 P(x^2)/Q(x^2)
   for  |x| <= 0.66
*/

static double P[5] = {
		      -8.750608600031904122785E-1,
		      -1.615753718733365076637E1,
		      -7.500855792314704667340E1,
		      -1.228866684490136173410E2,
		      -6.485021904942025371773E1,
};
static double Q[5] = {
		      /* 1.000000000000000000000E0, */
		      2.485846490142306297962E1,
		      1.650270098316988542046E2,
		      4.328810604912902668951E2,
		      4.853903996359136964868E2,
		      1.945506571482613964425E2,
};

double atan(double x)
{
	double sign = copysign(_fone, x);
	double z;
	double y = 0;
	x = fabs(x);
	if (x > 2.4) {
		y = _pi_over_2;
		x = _fminus / x;
	} else if (x > 0.6) {
		y = _pi_over_4;
		x = (x - _fone) / (x + _fone);
	}
	z = x * x;
	z = z * _polevl(z, P, 4) / _p1evl(z, Q, 5);
	x = x * z + x;
	return copysign(y + x, sign);
}


