#include <string.h>
#include <stdio.h>
#include <gigatron/console.h>

#define _CONS_STDIO
#include "_stdio.h"

static char cons_ibuf[CONS_BUFSIZE];

static int cons_write(FILE *fp, register const char *buf, register size_t cnt)
{
	return console_print((char*)buf, cnt);
}

static int cons_read(FILE *fp, register char *buf, size_t cnt)
{
	if (fp->_flag & _IOFBF) {
		register int n;
		while (! (n = _sread(fp, buf, cnt))) {
			fp->_x = cons_ibuf;
			console_readline(cons_ibuf, CONS_BUFSIZE);
		}
		return n;
	} else {
		*(char*)buf = (char)console_waitkey();
		return 1;
	}
}

struct _iovec _cons_iovec = { cons_read, cons_write };
