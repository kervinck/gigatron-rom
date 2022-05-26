#include <gigatron/libc.h>
#include <errno.h>
#include <math.h>

static double one_over_ln10 = 0.43429448190325176;

double log10(double x)
{
	return log(x) * one_over_ln10;
}
