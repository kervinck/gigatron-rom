#include <stdio.h>
#include <string.h>
#include <errno.h>

void perror(register const char *s)
{
	register FILE *f = stderr;
	if (s && *s) {
		fputs(s, f);
		fputs(": ", f);
	}
	fputs(strerror(errno), f);
	fputc('\n', f);
}
