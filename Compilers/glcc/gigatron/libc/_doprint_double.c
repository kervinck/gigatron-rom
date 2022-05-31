#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

#include "_doprint.h"

void _doprint_double_imp(doprint_t *dd, doprintspec_t *spec, __va_list *ap)
{
	char buffer[64];
	double x = va_arg(*ap, double);
	register int f = spec->flags;
	register int prec = spec->prec;
	register char *s;
	
	if (! (f & DPR_PREC))
		prec = 6;
	s = dtoa(x, buffer, spec->conv | 0x20, prec);
	spec->flags = f & ~DPR_PREC;
	_doprint_num(dd, spec, 129, s);
}
