#include <stdlib.h>
#include <math.h>
#include <gigatron/console.h>

char buffer[80];

int main()
{
	const char *s = "pi";
	double x = 3.141592653589793;
	double y;
	for(;;) {
		cprintf("\fGigatron floating point\n\n");
		cprintf("- %s=%.8g\n", s, x);
		y = sqrt(x);
		cprintf("- y=sqrt(%s)=%.8g\n", s, y);
		cprintf("- y*y=%.8g\n", y*y);
		cprintf("- log(%s)=%.8g\n", s, log(x));
		cprintf("- log(y)=%.8g\n", log(y));
		s = "x";
		cprintf("\nYour number? ");
		console_readline(buffer,sizeof(buffer));
		x = atof(buffer);
	}
}
