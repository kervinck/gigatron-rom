
#include <ctype.h>
#include <stdio.h>



int c(int x)
{
	if (isprint(x))
		return x;
	return '?';
}

int main()
{
	int i;
	for (i=0; i<256; i++)
		printf("[%c] %02x [%c] %02x [%c] %02x\n",
		       c(i),i,c(tolower(i)),tolower(i), c(toupper(i)), toupper(i) );
	return 0;
}
