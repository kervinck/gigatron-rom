#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

#include "_doscan.h"

int _doscan_double_imp(doscan_t *dd, double *p)
{
	strtod_t dobj;
	register strtod_t *d = &dobj;
	memset(d, 0, sizeof(*d));
	while (_strtod_push(d, dd->c, 0)) {
		_doscan_next(dd);
	}
	return _strtod_decode(d, p);
}
