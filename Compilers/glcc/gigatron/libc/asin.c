#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <gigatron/libc.h>

/* From Cephes:
/* arcsin(x)  =  x + x^3 P(x^2)/Q(x^2)
   for |x| <= 0.625 
*/
static double P[6] = {
		      4.253011369004428248960E-3,
		      -6.019598008014123785661E-1,
		      5.444622390564711410273E0,
		      -1.626247967210700244449E1,
		      1.956261983317594739197E1,
		      -8.198089802484824371615E0,
};
static double Q[5] = {
		      /* 1.000000000000000000000E0, */
		      -1.474091372988853791896E1,
		      7.049610280856842141659E1,
		      -1.471791292232726029859E2,
		      1.395105614657485689735E2,
		      -4.918853881490881290097E1,
};

static double k_asin(double x) /* for |x|<0.625 */
{
	double z = x * x;
	z = z * _polevl(z, P, 5) / _p1evl(z, Q, 5);
	return x * z + x;
}

/* cos(2y) = 1 - 2 sin^2(y)
   Therefore is y = arcsin(sqrt((1-x)/2)) then cos(2y) = x.
   When x > 0.5, arccos(x) = 2 arcsin(sqrt((1-x)/2)) */

static double k_acos(double x) 	/* For x > 0.5 */
{
	return ldexp(k_asin(sqrt(ldexp(_fone - x, -1))), 1);
}

/* Finally acos(x) + asin(x) = pi/2 always */

double asin(double x)
{
	double sign = copysign(_fone, x);
	x = fabs(x);
	if (x > _fone) {
		errno = EDOM;
		return _fexception(_fzero);
	} else if (x > _fhalf) {
		x = _pi_over_2 - k_acos(x);
	} else 
		x = k_asin(x);
	return copysign(x, sign);
}

double acos(double x)
{
	int sign = (x < _fzero);
	x = fabs(x);
	if (x > _fone) {
		errno = EDOM;
		return _fexception(_fzero);
	} else if (x > _fhalf) {
		x = k_acos(x);
	} else
		x = _pi_over_2 - k_asin(x);
	if (sign)
		x = _pi - x;
	return x;
}
