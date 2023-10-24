#include <stdio.h>
#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <string.h>
#include <gigatron/libc.h>

/* Helpers defined in itoa.s */
extern int _utwoa(int);
extern char *_uftoa(double,char*);

#ifdef DEBUG_DTOA
# define DBG(x) printf x
#else
# define DBG(x)
#endif

static const double halfm = (2.0 - DBL_EPSILON) / 4.0;
static const double billion = 1e9;

char *dtoa(double x, char *buf, register char fmt, register int prec)
{
	int tmp;
	char lbuf[16];
	register int exp, nd, per;
	register char *q = buf;
	register char *s;
	(void) &x;

	if (x < _fzero) {
		*q = '-';
		q++;
	}
	/* Decode */
	x = _frexp10(fabs(x), &tmp) + halfm;
	nd = 9;
	if (x >= billion)
		nd = 10;
	else if (x < _fone)
		nd = 1;
	DBG(("| frexp -> %.0f exp=%d nd=%d\n", x - halfm, tmp, nd));
	/* Position period */
	per = 1;
	exp = tmp + nd - per;
	if (fmt == 'g' && prec - 1 > 0)
		prec -= 1;
	if (fmt == 'f' || fmt != 'e' && exp + 4 >= 0 && exp - prec <= 0) {
		if ((per = per + exp) != 1 && (fmt != 'f'))
			prec = prec - per + 1;
		exp = 0;
	}
	DBG(("| exp=%d per=%d prec=%d nd=%d\n", exp, per, prec, nd));
	/* Truncate */
	if (per + prec - nd < 0) {
		x = _ldexp10(x - halfm, per + prec - nd) + halfm;
		nd = per + prec;
		DBG(("| trunc: nd=%d\n", nd));
	}
	/* Extract digits */
	//s = ultoa((long)x, lbuf, 10);
	s = _uftoa(x, lbuf);
	DBG(("| digits=[%s]\n", s));
	if (s[nd] != 0) {
		/* Carry propagation during rounding got us an extra digit */
		if (fmt == 'e' || fmt != 'f' && exp != 0) {
			s[nd] = 0;
			exp += 1;
		} else
			per += 1;
		DBG(("| carry adjustment: digits=[%s] exp=%d per=%d\n", s, exp, per));
	}
	/* Output digits */
	if ((nd = 1 - per) < 0)
		nd = 0;
	per = per + nd;
	DBG(("| skip=%d per=%d\n", nd, per));
	while (per > 0 || prec > 0) {
		if (per == 0) {
			*q = '.';
			q += 1;
		}
		if ((per = per - 1) < 0)
			prec = prec - 1;
		if (nd <= 0 && *s) {
			*q = *s;
			s += 1;
		} else
			*q = '0';
		q += 1;
		nd -= 1;
	}
	/* Remove zeroes for g style */
	if (fmt == 'g' && per < 0) {
		do {
			q -= 1;
		} while (*q == '0');
		if (*q != '.')
			q += 1;
	}
	/* Output exponent */
	if (exp || fmt == 'e') {
		*q = 'e';
		q += 1;
		if (exp >= 0)
			*q = '+';
		else {
			exp = -exp;
			*q = '-';
		}
		q += 1;
		exp = _utwoa(exp);
		*q = (exp >> 8);
		q += 1;
		*q = exp;
		q += 1;
	}
	*q = 0;
	return buf;
}

