#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <gigatron/libc.h>
#include <gigatron/sys.h>





void bg()
{
	int i;
	clrscr();
	SYS_Fill(makew(8,0), 0x10, makew(120,160));
	for (i=0; i!=120; i+=10)
		SYS_Fill(makew(8+i,0), 0x20, makew(5,160));
	for (i=0; i!=160; i+=10)
		SYS_Fill(makew(8,i), 0x30, makew(120,1));
}


void rect(int x, int y, int w, int h, int c)
{
	unsigned addr = makew(8+y,x);
	SYS_Fill(addr, c, makew(1,w));
	SYS_Fill(makew(8+y+h-1,x), c, makew(1,w));
	SYS_Fill(addr, c, makew(h,1));
	SYS_Fill(makew(8+y,x+w-1), c, makew(h,1));
}

void target(int x, int y, int w, int h)
{
	while (w > 0 && h > 0) {
		rect(x, y, w, h, SYS_Random() & 0x3f);
		x ++;
		y ++;
		w -= 2;
		h -= 2;
	}
}

void go(int dx, int dy, int sx, int sy, int w, int h)
{
	bg();
	gotoxy(1,1);
	cprintf("%dx%d+%d,%d -> %d,%d", w, h, sx, sy, dx, dy);
	rect(10,10,140,100,0x3f);
	target(sx, sy, w, h);
	SYS_Blit( makew(8+dy, dx), makew(8+sy,sx), makew(h,w));
}



int main()
{
	go(50, 10, 10, 50, 30, 40); getch();
	go(50, 50, 10, 10, 11, 36); getch();
	go(50, 10, 100, 50, 32, 32); getch();

	go(40, 20, 50, 30, 60, 30); getch();
	go(40, 40, 50, 30, 60, 30); getch();
	go(60, 20, 50, 30, 60, 30); getch();
	go(60, 40, 50, 30, 60, 30); getch();

	go(20, 10, 90, 10, 60, 85); getch();
	go(100, 10, 30, 10, 60, 85); getch();
	go(40, 10, 50, 10, 60, 85); getch();
	go(60, 10, 50, 10, 60, 85); getch();
	
	return 0;
}
