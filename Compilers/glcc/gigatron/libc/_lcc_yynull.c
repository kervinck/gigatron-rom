
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <gigatron/libc.h>

#define LEN 26

static char msg[LEN+1];
static char buf[8];

void _YYnull(register char *file, register int line)
{
	register char *d = msg;
	register char *s = strrchr(file, '/');
	register int i;
	char *x[4];
	x[0] = (d) ? d + 1 : file;
	x[1] = ":";
	x[2] = utoa(line, buf, 10);
	x[3] = ": null check";
	for (i = 0; i != 4; i++)
		for (s=x[i]; *s; s++)
			if (d != msg+LEN)
				*d++ = *s;
	*d++ = 0;
	_assertfail(msg);
}
