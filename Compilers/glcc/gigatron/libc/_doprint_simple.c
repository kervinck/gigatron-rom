#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

#include "_doprint.h"

static char hash1[8] = "uiocdxss";
static char hash2[9] = { 10, 255, 8, 0, 255, 16, 1, 1 };


int _doprint_simple(register const char *f, register __va_list ap)
{
	register const char *s;
	register int l, c, pad;
	register char style;
	char buf8[8];

	_doprint_dst.cnt = 0;
	while(*f) {
		s = f;
		style = ' ';
		pad = 0;
		if (*f != '%') {
			while (*f && *f != '%')
				f++;
			l = f - s;
		} else {
			c = *++f;
			if (c == '-' || c == '0') {
				style = c;
				c = *++f;
			}
			while(_isdigit(c)) {
				pad = pad * 10 + c - 0x30;
				c = *++f;
			}
			f += 1;
			/* perfect hash */
			l = c;
			if (l - 110 > 0)
				l = l ^ 5;
			l = l & 7;
			if (c != hash1[l]) {
				/* not one of uiocdxs */
				if (c != '%')
					f = s + 1;
				pad = 0;
				l = 1;
			} else {
				c = hash2[l];
				l = va_arg(ap, int);
				if (c == 0) {               /* %c */
					s = buf8;
					*(char*)s = l;
					l = 1;
				} else if (c == 1) {        /* %s */
					s = (char*)l;
					l = strlen(s);
				} else {
					if (c == 255) {     /* %i %d */
						s = itoa(l, buf8, 10);
						if (style == '0' && *s == '-') {
							_doprint_putc('-', 1);
							pad -= 1;
							s += 1;
						}
					} else {            /* %u %o %x */
						s = utoa((unsigned int)l, buf8, c);
					}
					l = buf8 + 7 - s;
				}
				pad -= l;
			}
		}
		if (style != '-' && pad > 0)
			_doprint_putc(style, pad);
		_doprint_puts(s, l);
		if (style == '-' && pad > 0)
			_doprint_putc(' ', pad);
	}
	return _doprint_dst.cnt;
}
