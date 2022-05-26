#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <gigatron/console.h>
#include <gigatron/libc.h>


char buffer[80];
char buf2[64];

void pr(const char *s1, const char *s2, const char *s3, double d)
{
	
	strcpy(buffer,s1);
	if (s2) strcat(buffer, s2);
	if (s3) strcat(buffer, s3);
	strcat(buffer, dtoa(d, buf2, 'g', 6));
	strcat(buffer, "\n");
	console_print(buffer, 128);
}

int main()
{
	const char *s = "pi";
	double x = 3.141592653589793;
	double y;
	
	for(;;) {
		console_print("\fGigatron floating point\n\n", 999);
		pr("- ", s, "=", x);
		y = sqrt(x);
		pr("- y=sqrt(", s, ")=", y);
		pr("- y*y=", 0, 0, y*y);
		pr("- log(", s, ")=", log(x));
		pr("- log(y)=", 0, 0, log(y));
		s = "x";
		console_print("\nYour number? ", 999);
		console_readline(buffer,128);
		x = atof(buffer);
	}
}
