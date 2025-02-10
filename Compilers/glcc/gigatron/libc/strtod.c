#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <string.h>
#include <gigatron/libc.h>

#include "_stdlib.h"

/* This code is much simpler than a numerically correct strtod 
   but gets the exact numbers right. */

#define FLG_STATE  3
# define ST_SIGN    0
# define ST_MANT    1
# define ST_EXPSGN  2
# define ST_EXPDIG  3
#define FLG_ENEG   8
#define FLG_DIGIT  16
#define FLG_PERIOD 32
#define FLG_NEG    64
#define FLG_OVF    128

int _strtod_push(strtod_t *d, int c, const char *p)
{
	/* lookahead characters p[0] and p[1] are only
	   used to optionally validate the exponent. */
	register int f = d->flags;
	
	if (f == 0) {
		f = ST_MANT;
		if (c == '-') {
			f |= FLG_NEG;
			goto ret;
		} else if (c == '+')
			goto ret;
	}
	if ((f & FLG_STATE) == ST_MANT) {
		if (c == '.') {
			if (f & FLG_PERIOD)
				goto end;
			f |= FLG_PERIOD;
			goto ret;
		} else if (_isdigit(c)) {
			double x = d->x;
			f |= FLG_DIGIT;
			if (x < 1e16) {
				if (f & FLG_PERIOD)
					d->e0 -= 1;
				d->x = x * 10 + (double)(c - '0');
			} else if (! (f & FLG_PERIOD)) 
					d->e0 += 1;
			goto ret;
		} else if ((c | 0x20) == 'e') {
			f = f & (FLG_STATE ^ 0xff) | ST_EXPSGN;
			if (p) {
				c = p[1];
				if (c == '+' || c == '-')
					c = p[2];
				if (!_isdigit(c))
					goto end;
			}
			goto ret;
		}
	}
	if ((f & FLG_STATE) == ST_EXPSGN) {
		f = f & (FLG_STATE ^ 0xff) | ST_EXPDIG;
		if (c == '-') {
			f |= FLG_ENEG;
			goto ret;
		} else if (c == '+')
			goto ret;
	}
	if ((f & FLG_STATE) == ST_EXPDIG) {
		if (_isdigit(c)) {
			register int e1 = d->e1;
			if (e1 < 250)
				d->e1 = e1 * 10 + c - '0';
			goto ret;
		}
	}
 end:
	return 0;
 ret:
	return d->flags = f;
}

int _strtod_decode(strtod_t *d, double *px)
{
	double x = d->x;
	register int e = d->e0;
	register int f = d->flags;
	void *saved_raise_disposition = _raise_disposition;

	if (! (f & FLG_DIGIT))
		return 0;
	if (f & FLG_ENEG)
		e = e - d->e1;
	else
		e = e + d->e1;
	_raise_code = 0;
	_raise_disposition = RAISE_SETS_CODE;
	x = _ldexp10p(&x, e);
	if (_raise_code) {
		x = HUGE_VAL;
		errno = ERANGE;
	}
	_raise_disposition = saved_raise_disposition;
	if (f & FLG_NEG)
		x = -x;
	if (px)
		*px = x;
	return 1;
}

double strtod(const char *nptr, char **endptr)
{
	strtod_t dobj;
	double x = 0.0;
	register strtod_t *d = &dobj;
	register const char *p = nptr;

	memset(d, 0, sizeof(dobj));
	while (isspace(p[0]))
		p += 1;
	while (_strtod_push(d, p[0], &p[1]))
		p += 1;
	if (! _strtod_decode(d, &x))
		p = nptr;
	if (endptr)
		*endptr = (char*) p;
	return x;
}

double atof(register const char *s)
{
	return strtod(s, NULL);
}



