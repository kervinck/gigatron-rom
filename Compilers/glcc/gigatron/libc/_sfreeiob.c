#include "_stdio.h"
#include "errno.h"

/* This is a weak reference to function 'free' */
extern void __glink_weak_free(void*);

void  _sfreeiob(FILE *fp)
{
	if ((fp->_flag & _IOMYBUF) && fp->_base)
		__glink_weak_free(fp->_base);
	fp->_base = 0;
	fp->_flag = 0;
	fp->_v = 0;
}
