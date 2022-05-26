#include <stdlib.h>
#include <stdio.h>
#include <math.h>

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
		printf("\nYour number? ");
		scanf("%f%*[^\r\n]", &x);
	}
}
