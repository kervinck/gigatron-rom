#include <stdlib.h>
#include <string.h>
#include <gigatron/console.h>


const char *itoa(int x)
{
	int neg = 0;
	static char buffer[8];
	register char *s = buffer+sizeof(buffer);
	*s-- = 0;
	if (x < 0)
		neg = x = -x;
	do {
		*--s = x % 10 + '0';
		x = x / 10;
	} while (x > 0);
	if (neg)
		*--s = '-';
	return s;
}


int main()
{
	int i;
	char buf[80];
	console_print("\tHello World!\n\a(bell)\n", 256);
	for (i=0; i<20; i++) {
		console_print(itoa(i), 256);
		console_print("\n", 1);
	}

	while (i != 'q') {
		console_print("Key: ", 5);
		i = console_waitkey();
		console_print(itoa(i), 256);
		strcpy(buf,"[?]\n");
		if (i >= 0x20 && i <= 0x84)
			buf[1] = i;
		console_print(buf,4);
	}

	do {
		console_print("Line: ", 6);
		console_readline(buf, sizeof(buf));
		console_print("Read: ", 6);
		console_print(buf, sizeof(buf));
	} while (strcmp(buf,"q\n"));

	abort();
	return 0;
}
