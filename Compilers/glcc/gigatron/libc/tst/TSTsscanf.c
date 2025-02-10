#include <stdio.h>
#include <string.h>


const char *strings[] = {
	"12 13 14",
	" -12 -13 -14.3",
	"0x1234, rest",
	"  abcdefghijklmnopqrstuvwxyz",
	0
};

int main()
{
	int i;
	for (i=0; strings[i]; i++) {
		int i0,i1,i2,n,r;
		long l0,l1;
		double d2 = -1.0;
		char buf[50];
		const char *fmt;
		const char *s = strings[i];
		printf("=== string = \"%s\" ===\n", s);
		r = sscanf(s, fmt=" %s%n", buf, &n);
		printf("fmt=\"%s\" r=%d buf=\"%s\", n=%d\n", fmt, r, buf, n);
		r = sscanf(s, fmt=" %5s%n", buf, &n);
		printf("fmt=\"%s\" r=%d buf=\"%s\", n=%d\n", fmt, r, buf, n);
		memset(buf,0,sizeof(buf));
		r = sscanf(s, fmt=" %5c%n", buf, &n);
		printf("fmt=\"%s\" r=%d buf=\"%s\", n=%d\n", fmt, r, buf, n);
		memset(buf,0,sizeof(buf));
		r = sscanf(s, fmt=" %8[acbd]%n", buf, &n);
		printf("fmt=\"%s\" r=%d buf=\"%s\", n=%d\n", fmt, r, buf, n);
		memset(buf,0,sizeof(buf));
		r = sscanf(s, fmt=" %8[^ ]%n", buf, &n);
		printf("fmt=\"%s\" r=%d buf=\"%s\", n=%d\n", fmt, r, buf, n);
		r = sscanf(s, fmt=" %d %u %x%n", &i0, &i1, &i2, &n);
		printf("fmt=\"%s\" r=%d i0=%d i1=%d i2=%d, n=%d\n", fmt, r, i0, i1, i2, n);
		r = sscanf(s, fmt=" %i %u %x%n", &i0, &i1, &i2, &n);
		printf("fmt=\"%s\" r=%d i0=%d i1=%d i2=%d, n=%d\n", fmt, r, i0, i1, i2, n);
		r = sscanf(s, fmt="%d %u %x%n", &i0, &i1, &i2, &n);
		printf("fmt=\"%s\" r=%d i0=%d i1=%d i2=%d, n=%d\n", fmt, r, i0, i1, i2, n);
		r = sscanf(s, fmt=" %ld %ld %lf%n", &l0, &l1, &d2, &n);
		printf("fmt=\"%s\" r=%d l0=%ld l1=%ld d2=%.4f n=%d\n", fmt, r, l0, l1, d2, n);
	}

	return 0;
}
