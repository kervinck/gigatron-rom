#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <gigatron/libc.h>

double atan2(double y, double x)
{
	double sx = copysign(_fone, x);
	double sy = copysign(_fone, y);
	x = fabs(x);
	y = fabs(y);
	if (x == 0 && y == 0)
		x = 0;
	else if (y <= x)
		x = atan(y / x);
	else
		x = _pi_over_2 - atan(x / y);
	if (sx < _fzero)
		x = _pi - x;
	if (sy < _fzero)
		x = -x;
	return x;
}
