#include <gigatron/libc.h>
#include <errno.h>
#include <math.h>



static double R[3] = {
		      -7.89580278884799154124E-1,
		      1.63866645699558079767E1,
		      -6.41409952958715622951E1,
};
static double S[3] = {
		      /* one */
		      -3.56722798256324312549E1,
		      3.12093766372244180303E2,
		      -7.69691943550460008604E2,
};

static double LOG2 = 0.6931471805599453;
static double SQRTH = 0.70710678118654752440;

double log(register double x)
{
	int e;
	register double z;
	if (x <= 0) {
		errno = EDOM;
		return _fexception(-HUGE_VAL);
	}
	x = frexp(x, &e);
	if (x < SQRTH) {
		x = ldexp(x,1);
		e -= 1;
	}
	z = x - _fhalf;
	x = (z - _fhalf) / (_fhalf * x + _fhalf);
	z = x * x;
	z = x * ( z * _polevl( z, R, 2 ) / _p1evl( z, S, 3 ) );
	return x + z + e * LOG2;
}
