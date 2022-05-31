#include <math.h>
#include <errno.h>
#include <gigatron/libc.h>

double sqrt(double x)
{
	if (x <= _fzero) {
		if (x == _fzero)
			return _fzero;
		errno = EDOM;
		return _fexception(_fminus);
	} else {
		int e;
		register double z = frexp(x, &e);
		register int i;
		z = 4.173075996388649989089E-1 + 5.9016206709064458299663E-1 * z;
		if (e & 1)
			z = z * 1.41421356237309504880;
		z = ldexp(z, (e >> 1));
		for (i=3; i; i--)
			z = (z + x / z) * _fhalf;
		return z;
	}
}

