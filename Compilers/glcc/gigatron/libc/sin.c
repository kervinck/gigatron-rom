#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <gigatron/libc.h>

/* Constants from the cephes library
 *
 * For -pi/4 <= x <= pi/4:
 *    sin(x) = x + x^3 * polevl(x^2, sincof, 5)
 *    cos(x) = 1 - x^2/2 + x^4 * polevl(x^2, coscof, 5)
 */

static double sincof[] = {
 1.58962301576546568060E-10,
-2.50507477628578072866E-8,
 2.75573136213857245213E-6,
-1.98412698295895385996E-4,
 8.33333333332211858878E-3,
-1.66666666666666307295E-1,
};

static double coscof[6] = {
-1.13585365213876817300E-11,
 2.08757008419747316778E-9,
-2.75573141792967388112E-7,
 2.48015872888517045348E-5,
-1.38888888888730564116E-3,
 4.16666666666665929218E-2,
};

static double k_sin_over_x(double x) {
	x *= x;
	return _fone + _polevl(x, sincof, 5) * x;
}

static double k_cos(double x) {
	x *= x;
	return _fone - _fhalf * x + _polevl(x, coscof, 5) * x * x; 
}



static double sin_e(double x, int add)
{
	int tmp;
	register int quo;
	x = _fmodquo(x, _pi_over_4, &tmp);
	if (x < _fzero) {
		quo = (add - tmp - 1);
		x = _pi_over_4 + x;
	} else
		quo = add + tmp;
	if (quo & 1)
		x = _pi_over_4 - x;
	if ((quo + 1) & 2)
		x = k_cos(x);
	else
		x = k_sin_over_x(x) * x;
	if (quo & 4)
		x = -x;
	return x;
}

double sin(double x)
{
	return sin_e(x, 0);
}

double cos(double x)
{
	return sin_e(x, 2);
}

double tan(double x)
{
	return sin(x) / cos(x);
}
