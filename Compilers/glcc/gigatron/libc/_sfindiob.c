#include "_stdio.h"
#include "errno.h"

/* A library that provides file support may 
   contain a variant of this function
   compiled with -DWITH_MALLOC=1. */

FILE *_sfindiob(void)
{
	register FILE *f = _iob;
#if WITH_MALLOC
	register struct _more_iobuf **pnext = &_more_iob;
	for(;;) {
#endif
		register int i;
		for (i = 0; i != _IOB_NUM; i++, f++)
			if  (! f->_flag)
				return f;
#if WITH_MALLOC
		if (! *pnext)
			*pnext = calloc(1, sizeof(struct _more_iob));
		if (! *pnext)
			break;
		f = (*pnext)->_iob;
		pnext = &(*pnext)->next;
	}
#endif
	errno = ENFILE;
	return 0;
}


