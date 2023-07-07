#include <stdio.h>


#ifdef __gigatron__
typedef unsigned int word;
typedef int sword;
#else
typedef unsigned short word;
typedef short sword;
#endif

int main()
{
	unsigned int i;
	int j;

	for (i=0; i<0x8000u; i=(i+i)^0x45) {
		printf("\n");
#define TEST(j) do {\
  printf("%04x >> %04x = %04x\t", i, j, (word)(i >> j));\
  printf("%04x >> %04x = %04x\t", i, j, (word)((sword)i >> j));\
  printf("%04x >> %04x = %04x\n", (word)(-(sword)i), j, (word)((-(sword)i) >> j));\
} while(0)

# define TST4(j) TEST(j+0);TEST(j+1);TEST(j+2);TEST(j+3)
		TST4(0);TST4(4);TST4(8);TST4(12);
		for (j=0; j<16; j++)
			TEST(j);
	}
	return 0;
}
