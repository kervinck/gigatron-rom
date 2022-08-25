#include <string.h>
#include <stdio.h>
#include <gigatron/console.h>

#define _CONS_STDIO
#include "_stdio.h"


struct _cbuf { 
	int size;
	char xtra[2];
	char data[CONS_BUFSIZE];
};

static struct _cbuf _cons_ibuf = { CONS_BUFSIZE };

static int cons_write(FILE *fp, register const void *buf, register size_t cnt)
{
	return console_print((char*)buf, cnt);
}

static int cons_flsbuf(register int c, register FILE *fp)
{
	fp->_ptr = 0;
	fp->_cnt = 0;
	if (c >= 0) {
		fp->_buf[0] = c;
		console_print(fp->_buf, 1);
		return c;
	}
	return 0;
}

static int cons_read(FILE *fp, register void *buf, size_t cnt)
{
	*(char*)buf = (char)console_waitkey();
	return 1;
}

static int cons_filbuf(register FILE *fp)
{
	register int n;
	register char *buf = _cons_ibuf.data;
	fp->_base = (struct _sbuf*)&_cons_ibuf;
	if (fp == stdin)
		_fflush(stdout);
	if (fp->_flag & _IOFBF)
		n = console_readline(buf, CONS_BUFSIZE);
	else
		n = cons_read(fp, buf, 1);
	fp->_cnt = n - 1;
	fp->_ptr = buf + 1;
	return buf[0];
}

struct _svec _cons_svec = { cons_flsbuf, cons_write, cons_filbuf, cons_read, 0, 0 };
