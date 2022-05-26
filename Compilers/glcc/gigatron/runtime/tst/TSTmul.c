#include <stdio.h>

int main()
{
	int i, j;

	for (i=0; i<10; i++) {
		printf("\n");
		for (j=0; j<10; j++) {
			printf("%d * %d = %d\t", i, j, i * j);
			printf("%d * %d = %d\n", i, -j, i * (-j));			
		}
	}
	for (i=0; i<10; i++) {
		printf("\n");
		for (j=90; j<100; j++) {
			printf("%d * %d = %d\t", i, j, i * j);
			printf("%d * %d = %d\n", -i, j, (-i) * j);
		}
	}
	return 0;
}
