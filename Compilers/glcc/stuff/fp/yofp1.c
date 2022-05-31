#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <gigatron/console.h>

char buffer[80];

int main()
{
	const char *s = "pi";
	double x = 3.141592653589793;
	double y;
	for(;;) {
		printf("\fGigatron floating point\n\n");
		printf("- %s=%.8g\n", s, x);
		y = sqrt(x);
		printf("- y=sqrt(%s)=%.8g\n", s, y);
		printf("- y*y=%.8g\n", y*y);
		printf("- log(%s)=%.8g\n", s, log(x));
		printf("- log(y)=%.8g\n", log(y));
		s = "x";
		console_print("\nYour number? ", 999);
		console_readline(buffer,128);
		x = atof(buffer);
	}
}
