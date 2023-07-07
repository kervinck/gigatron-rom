#include "stdio.h"


/* This initialization function is called before main whenever _iob[]
   is referenced in the program. The default version hooks stdin,
   stdout, and stderr to the console. */

extern struct _iovec _sim_iovec;

void _iob_setup(void)
{
	_iob[0]._flag = _IOLBF|_IOREAD;
	_iob[1]._flag = _IOFBF|_IOWRIT;
	_iob[2]._flag = _IONBF|_IOWRIT;
	_iob[0]._v = &_sim_iovec;
	_iob[1]._v = &_sim_iovec;
	_iob[2]._v = &_sim_iovec;
	_iob[0]._x = (void*)0;
	_iob[1]._x = (void*)1;
	_iob[2]._x = (void*)2;
}

