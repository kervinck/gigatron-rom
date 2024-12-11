
#include <stdlib.h>
#include <gigatron/pragma.h>
#include <gigatron/sys.h>

#ifdef STATS
#include <conio.h>
#include <gigatron/libc.h>
#endif


#if _GLCC_VER < 204009
# error "Need GLCC >= 2.4.9"
#endif

extern void table(void);
extern char SYS_Random_34[];

#define CENTERX 80
#define CENTERY 60
#define ADDR(x,y) ((x + CENTERX) | ((y + CENTERY + 8)<<8))

static void rotate(void)
{
	sysFn = (word)SYS_Random_34;
	for(;;) {
#ifdef STATS
		int i;
		unsigned int c = _clock();
		for (i=0; i != 30; i++)
#endif
			(*table)();
#ifdef STATS
		c = 18000 / (_clock() - c);
		gotoxy(1,1);
		cprintf("%d.%0d/s", c/10, c%10);
		sysFn = (word)SYS_Random_34;
#endif
	}
}

#ifndef STATS
static void clrscr(void)
{
	char *p;
	for (p = (char*)0x800; p != (char*)0x8000; p += 256)
		SYS_SetMemory(160, 0, p);
}
#endif

int main()
{
	SYS_SetMode(3);
	clrscr();
	rotate();
	return 0;
}



