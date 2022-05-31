#include <stdio.h>
#include <stdlib.h>

#if DEBUG
extern void malloc_map(void);
#else
# define malloc_map() /**/
#endif


void *p[100];
int   s[100];
int   r[100];


void cycle(int n)
{
	int i,d,t;
	printf("Testing with %d blocks\n", n);
	for (i=0; i!=n; i++) {
		do { d = rand() & 0x7f; } while (d < 2);
		s[i] = d = 65535 / (d * d);
		p[i] = malloc(d);
		r[i] = i;
		printf("- Allocated %d bytes at ptr%d\n", d, i);
	}
	malloc_map();
	for (i=0; i!=n; i++) {
		d = rand() % (n - i);
		t = r[i];
		r[i] = r[i+d];
		r[i+d] = t;
	}
	for (i=0; i!=n; i++) {
		d = r[i];
		printf("- Freeing ptr%d (size %d)\n", d, s[d]);
		free(p[d]);
	}
	malloc_map();
}
	




int main()
{
	int r = 4;
	srand(314);
	malloc_map();
	while (r <= 100) {
		cycle(r);
		r = r + (r >> 1);
	}
	return 0;
}
