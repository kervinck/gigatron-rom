#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <gigatron/console.h>
#include <gigatron/libc.h>

int mincprintf(const char *fmt, ...)
{
	register va_list ap;
	register const char *f = fmt;
	va_start(ap, fmt);
	
	while(*f) {
		register const char *s;
		register int l;
		if (*f != '%') {
			s = f;
			while (*f && *f != '%')
				f++;
			l = f - s;
		} else {
			f += 1;
			l = *f;
			if (l == 's') {
				s = va_arg(ap, char*);
				l = 32767;
			} else if (l == 'd') {
				char buf[8];
				s = itoa(va_arg(ap, int), buf, 10);
				l = 8;
			} else
				continue;
			f++;
		}			
		console_print(s, l);
	}
	return 0;
}
