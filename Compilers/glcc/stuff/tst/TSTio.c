#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <ctype.h>


int main()
{
	int c;
	char buf[8];
	fputs("Hello World!\n\n", stdout);
	while ((c = getchar()) != EOF)
		{
			fputs("Read [", stdout);
			putchar(isprint(c) ? c : '?');
			fputs("]\n", stdout);
			if (c == 'q')
				break;
			if (isdigit(c))
				ungetc('#', stdin);
			if (c == '0')
				ungetc('O', stdin);
		}
	return 0;
}
