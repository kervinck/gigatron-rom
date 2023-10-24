#include "_stdio.h"
#include <gigatron/console.h>

static struct _iovec _iov1 = {
	(writall_t)console_writall, 0
};

struct _iobuf _iob1 = {  /* stdout */
	_IOLBF|_IOWRIT, 0, &_iov1, (void*)1
};

struct _iobuf _iob2 = { /* stderr */
	_IONBF|_IOWRIT, 0, &_iov1, (void*)2
};

