#include <string.h>
#include <stdio.h>

char buffer[1024];

void test(int offset, int val, int len)
{
	int i;
	printf("Trying memset(buffer+%d,%d,%d)\n", offset,val,len);
	for (i=0; i<sizeof(buffer);i++)
		buffer[i] = (i&0xff);
	memset(buffer+offset, val, len);
	for (i=0; i<sizeof(buffer);i++)
		{
			int expected = (i & 0xff);
			if (i >= offset && i < offset+len)
				expected = val;
			if (buffer[i] != expected)
				printf(" at buffer+%d: expected=%02x found=%02x\n", i, expected, buffer[i]);
		}
}

int main()
{
	test(34,0,12);
	test(34,65,12);
	test(34,63,255);
	test(34,63,256);
	test(128,63,257);
	test(256,63,757);
	return 0;
}
