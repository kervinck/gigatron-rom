#include <gigatron/libc.h>
#include <errno.h>
#include <math.h>


static double _ipow(double x, double ye)
{
	register long i = ye;
	register double r = 1.0;
	while (i) {
		if (i & 1)
			r *= x;
		x *= x;
		i >>= 1;
	}
	return r;
}

double pow(double x, double y)
{
	double s = +1;
	double yf;
	double ye;
	int yi;
	
	if (y == 0.0 || x == 1.0)
		return 1.0;
	if (y < 0) {
		y = -y;
		x = 1.0 / x;
	}
	yf = modf(y, &ye);
	if (x <= 0) {
		if (x == 0)
			return 0;
		if (yf != 0) {
			errno = EDOM;
			return _fexception(0);
		}
		if ((int)ye & 1) {
			s = -1;
		}
		x = -x;
	}
	if (yf == 0 && ye < 1000)
		return s * _ipow(x, ye);
	return s * exp(y * log(x));
}
