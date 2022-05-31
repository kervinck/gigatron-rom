#include "stdio.h"


/* This initialization function is called before main whenever _iob[]
   is referenced in the program. The default version hooks stdin,
   stdout, and stderr to the console. */

extern struct _svec _sim_svec;
char ibuf[92];
char obuf[92];

void _iob_setup(void)
{
	/* stdin */
	_iob[0]._flag = _IONBF|_IOREAD;
	_iob[0]._v = &_sim_svec;
	_iob[0]._file = 0;
	setvbuf(&_iob[0], ibuf, _IOLBF, sizeof(ibuf));
	/* stdout */
	_iob[1]._flag = _IONBF|_IOWRIT;
	_iob[1]._v = &_sim_svec;
	_iob[1]._file = 1;
	setvbuf(&_iob[1], obuf, _IOLBF, sizeof(obuf));
	/* stderr */
	_iob[2]._flag = _IONBF|_IOWRIT;
	_iob[2]._v = &_sim_svec;
	_iob[2]._file = 2;
	/* buffer */
	
}

void _sim_flush(void)
{
	fflush(stdout);
}

