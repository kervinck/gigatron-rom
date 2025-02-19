#include <stdio.h>
#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <string.h>
#include <gigatron/libc.h>

/* Helpers defined in itoa.s */
extern char * _ftoa(double *x, char *bufend);
extern int _rnda(char *s, char *r);


#ifdef DEBUG_DTOA
# define DBG(x) printf x
#else
# define DBG(x)
#endif

char *dtoa(double x, char *buf, register int fmt, register int prec)
{
	char lbuf[16];
	register int exp, nd, per;
	register char *s, *q = buf;
	(void) &x;

	if (((char*)&x)[1] & 0x80) {
		((char*)&x)[1] &= 0x7f;
		*q = '-';
		q++;
	}
	/* Decode */
	exp = _frexp10p(&x);
	DBG(("| frexp -> %.0f exp=%d\n", x, exp));
	s = _ftoa(&x, lbuf+15);
	nd = lbuf + 15 - s;
	DBG(("| ftoa -> nd=%d s=[%.*s\\x%02x]\n", nd, nd, s, lbuf[15]));
	/* Position period */
	per = 1;
	exp = exp + nd - per;
	if (fmt == 'g' && prec - 1 > 0)
		prec -= 1;
	if (fmt == 'f' || fmt != 'e' && exp + 4 >= 0 && exp - prec <= 0) {
		per = per + exp;
		exp = 0;
		if (fmt != 'f')
			prec += 1 - per;
	}
	DBG(("| exp=%d per=%d prec=%d nd=%d\n", exp, per, prec, nd));
	/* Truncate */
	if (per + prec < 0)
		nd = 0;
	else if (per + prec - nd < 0)
		nd = per + prec;
	DBG(("| trunc: nd=%d ", nd));
	if (_rnda(s, s+nd)) {
		/* Carry propagation during rounding got us an extra digit */
		s -= 1;
		DBG((" s=[%s]\n", s));
		if (fmt == 'e' || fmt != 'f' && exp != 0) {
			s[nd] = 0;
			exp += 1;
		} else
			per += 1;
		DBG(("| carry adjustment: digits=[%s] exp=%d per=%d\n", s, exp, per));
	} else {
		DBG((" s=[%s]\n", s));
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

