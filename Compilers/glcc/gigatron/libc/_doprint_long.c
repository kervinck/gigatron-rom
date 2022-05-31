#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

#include "_doprint.h"

void _doprint_long_imp(doprint_t *dd, doprintspec_t *spec, int b, __va_list *ap)
{
	char buffer[16];
	register char *s;
	register unsigned long x = va_arg(*ap, unsigned long);
	if (b == 11) {
		s = ltoa(x, buffer, 10);
	} else
		s = ultoa(x, buffer, b);
	_doprint_num(dd, spec, b, s);
}
