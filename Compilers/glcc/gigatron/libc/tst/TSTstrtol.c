#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

const char * v[] =
	{ "23",
	  "-245",
	  "0x7fff",
	  "0x7FFF",
	  "0x7fff894",
	  "0x7fff8941",
	  "0x7fff89410",
	  "4294967295",
	  "4294967296",
	  "0377",
	  "+3000000000e10",
	  "-3000000000e10",
	  "83928038989809890809",
	  "0x1000000000000000fffff abcd",
	  "  -9999999999999",
	  "  23z",
	  "   asdf",
	  0 };

int main()
{
	long x;
	unsigned long u;
	const char **vv = v;
	const char *s;
	char *e;
	while (*vv) {
		s = *vv;
		printf("[%s]\n", s);
		errno = 0;
		u = strtoul(s, &e, 0);
		printf("\tstrtoul: %lu (%lx), delta=%+d errno=%d\n", u, u, e-s, errno);
		errno = 0;
		x = strtol(s, &e, 0);
		printf("\tstrtol:  %ld (%lx), delta=%+d errno=%d\n", x, x, e-s, errno);
		x = atol(s);
		printf("\tatol:    %ld (%lx)\n", x, x);
		x = atoi(s);
		printf("\tatoi:    %d (%x)\n", (int)x , (unsigned int)x);
		vv += 1;
	}
	return 0;
}

