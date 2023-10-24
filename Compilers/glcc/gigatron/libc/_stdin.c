#include "_stdio.h"
#include <gigatron/console.h>

#define CONS_BUFSIZE 80

static char cons_ibuf[CONS_BUFSIZE];

static int cons_read(FILE *fp, register char *buf, size_t cnt)
{
	register int c;
	register char **xptr = (char**)&fp->_x;
	if (! (fp->_flag & _IOFBF)) {
		*xptr = 0;
		c = console_waitkey();
	} else {
		while (!*xptr || !(c = **xptr)) {
			*xptr = cons_ibuf;
			console_readline(cons_ibuf, CONS_BUFSIZE);
		}
		*xptr += 1;
	}
	buf[0] = c;
	return 1;
}

static struct _iovec _iov0 = {
	0, cons_read,
};

struct _iobuf _iob0 = {  /* stdin */
	_IOLBF|_IOREAD, 0, &_iov0, (void*)0
};
