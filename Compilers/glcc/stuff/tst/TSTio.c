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
			if (c == 'U') {
				fputs("Read unbuffered\n", stdout);
				setvbuf(stdin, NULL, _IONBF, 0);
			}
			if (c == 'B') {
				fputs("Read buffered\n", stdout);
				setvbuf(stdin, NULL, _IOLBF, 0);
			}
			if (c == 'Q') {
				fputs("Quitting\n", stdout);
				break;
			}
			if (isdigit(c))
				ungetc('#', stdin);
			if (c == '0')
				ungetc('O', stdin);
		}
	return 0;
}
