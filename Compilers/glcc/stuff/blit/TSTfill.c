#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <gigatron/libc.h>
#include <gigatron/sys.h>



int rnd(int m)
{
	unsigned int x;
	do { x = SYS_Random() & 0xff; } while (x >= m);
	return x;
}





int main()
{
	unsigned int x1, y1, x2, y2, tmp;
	unsigned int addr, hw, c, pse;

	
	clrscr();
	for(;;) {
		pse = (serialRaw != 0xff);
		if (pse)
			clrscr();
		c = SYS_Random() & 0x3f;
		x1 = rnd(160);
		y1 = rnd(120);
		x2 = rnd(160);
		y2 = rnd(120);
		if (x1 > x2) { int tmp=x1; x1=x2; x2=tmp; }
		if (y1 > y2) { int tmp=y1; y1=y2; y2=tmp; }
		addr = makew(y1+8,x1);
		hw = makew(y2-y1+1,x2-x1+1);
		SYS_Fill(addr, c, hw);
		*(char*)addr = 3;
		*(char*)(addr + hw - 0x101) = 3;
		while (pse && serialRaw != 0xff)
			/**/;
	}
}
