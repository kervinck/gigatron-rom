#include "stdio.h"

extern struct _iovec _sim_iovec;

struct _iobuf _iob0 = {  /* stdin */
	_IOLBF|_IOREAD, 0, &_sim_iovec, (void*)0
};

struct _iobuf _iob1 = {  /* stdout */
	_IOLBF|_IOWRIT, 0, &_sim_iovec, (void*)1
};

struct _iobuf _iob2 = { /* stderr */
	_IONBF|_IOWRIT, 0, &_sim_iovec, (void*)2
};
