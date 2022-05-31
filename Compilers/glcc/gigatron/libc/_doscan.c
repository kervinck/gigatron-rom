#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

#include "_doscan.h"
#include "_stdlib.h"
#include "_stdio.h"

static void _doscan_init(register doscan_t *dd, FILE *fp)
{
	memset(dd, 0, sizeof(*dd));
	dd->fp = fp;
	dd->c = fgetc(fp);
}

int _doscan_next(register doscan_t *dd)
{
	dd->c = fgetc(dd->fp);
	dd->cnt += 1;
	return dd->c;
}

static void spc(register doscan_t *dd)
{
	while (isspace(dd->c))
		_doscan_next(dd);
}

static int do_str(doscan_t *dd, int conv, int len, char *set, char *p)
{
	register int c;
	while ((c = dd->c) > 0) {
		if (len == 0 ||
		    conv == 's' && isspace(c) ||
		    conv == '[' && !_bitset_test(set, c) )
			break;
		if (p) { 
			*p = c;
			p += 1;
		}
		len -= 1;
		_doscan_next(dd);
	}
	if (conv != 'c')
		*p = 0;
	return 1;
}

static int do_lng(register doscan_t *dd, int conv, register int cnvf, register void *p)
{
	register int sgnd;
	register int c;
	strtol_t dobj;
	register strtol_t *d = &dobj;
	register void *lp;
	long x;
	
	d->flags = 0;
	d->base = 0;
	if (conv == 'd') 
		d->base = 10;
	else if (conv == 'x')
		d->base = 16;
	else if (conv == 'o')
		d->base = 8;
	d->x = 0;
	while (_strtol_push(d, dd->c))
		_doscan_next(dd);
	lp = &x;
	if (cnvf & FLG_LONG)
		lp = p;
	if (sgnd = (conv == 'd' || conv == 'i'))
		c = _strtol_decode_s(d, lp);
	else
		c = _strtol_decode_u(d, lp);
	if (c && p && !(cnvf & FLG_LONG)) {
		*(int*)p = (int)x;
		if (sgnd && x != (long)(int)x ||
		    !sgnd && x != (unsigned long)(int) x)
			errno = ERANGE;
	}
	return c;
}

const char *parsespec(const char *fmt, unsigned int *lp, unsigned int *fp)
{
	register int c;

	*lp = 0;
	*fp = 0;
	c = *++fmt;
	if (c == '*') {
		*fp |= FLG_STAR;
		c = *++fmt;
	}
	while (c >= '0' && c <= '9') {
		*fp |= FLG_LEN;
		*lp = *lp * 10 + c - '0';
		c = *++fmt;
	}
	if (c == 'l' || c == 'L' || c == 'h') {
		if (c != 'h')
			*fp |= FLG_LONG;
		c = *++fmt;
	}
	if (! (*fp & FLG_LEN))
		*lp = (c == 'c') ? 1 : 0xffffu;
	return fmt;
}

const char *parseset(register const char *fmt, register char *set)
{
	register int c;
	register int neg = 0;
	_bitset_clear(set, 32);
	if ((c = *++fmt) == '^') {
		c = *++fmt;
		neg = 1;
	}
	while (c) {
		_bitset_set(set, c);
		if ((c = *++fmt) == ']')
			break;
	}
	if (neg)
		_bitset_compl(set, 32);
	return fmt;
}

int _doscan(register FILE *fp, register const char *fmt, register __va_list ap)
{
	unsigned int l, f;
	doscan_t ddd;
	char set[32];
	register doscan_t *dd = &ddd;
	register void *p;
	register int scnt;
	register int c;

	_doscan_init(dd, fp);
	for (; c = *fmt; fmt++) {
		if (isspace(c)) {
			spc(dd);
			continue;
		} else if (c != '%' || fmt[1] == '%' && fmt++) {
			if (dd->c != '%')
				break;
			_doscan_next(dd);
			continue;
		}
		fmt = parsespec(fmt, &l, &f);
		c = *fmt;
		p = 0;
		if (! (f & FLG_STAR))
			p = va_arg(ap, void*);
		if (c == 'n' && p) {
			*(int*)p = dd->cnt;
			continue;
		}
		if (c != 'c' && c != '[')
			spc(dd);
		scnt = dd->cnt;
		switch(c) {
		case '[':
			fmt = parseset(fmt, set); /* passthru */
		case 's': case 'c': 
			l = do_str(dd, c, l, set, p); break;
		case 'd': case 'i':
		case 'o': case 'u': case 'x': case 'p':
			l = do_lng(dd, c, f, p); break;
		case 'e': case 'f': case 'g':
			l = _doscan_double(dd, p); break;
		}
		if (dd->cnt > scnt && l) {
			dd->n += 1;
			continue;
		}
		break;
	}
	ungetc(dd->c, dd->fp);
	return dd->n;
}
