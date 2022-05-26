#include <string.h>
#include <stdio.h>

char sbuffer[1024];
char dbuffer[1024];

void test(int doff, int soff, int len)
{
	int i;
	printf("Trying memcpy(dbuffer+%d,sbuffer+%d,%d)\n", doff, soff, len);
	for (i=0; i<sizeof(sbuffer);i++)
		sbuffer[i] = (i&0x3f) | 0x80;
	for (i=0; i<sizeof(dbuffer);i++)
		dbuffer[i] = (i&0x3f) | 0x40;

	memcpy(dbuffer+doff, sbuffer+soff, len);

	for (i=0; i<sizeof(dbuffer);i++)
		{
			int expected = (i & 0x3f) | 0x40;
			if (i >= doff && i < doff + len)
				expected = ( (i-doff+soff) & 0x3f ) | 0x80;
			if (dbuffer[i] != expected)
				printf(" at dbuffer+%d: expected=%02x found=%02x\n", i, expected, dbuffer[i]);
			expected = (i & 0x3f) | 0x80;
			if (sbuffer[i] != expected)
				printf(" at sbuffer+%d: expected=%02x found=%02x\n", i, expected, sbuffer[i]);
		}
}

int main()
{
	test(255,256,257);
	test(34,0,12);
	test(34,65,12);
	test(84,63,255);
	test(34,63,256);
	test(128,256,257);
	test(256,63,757);
	return 0;
}

