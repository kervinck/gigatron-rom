#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gigatron/sys.h>
#include <gigatron/libc.h>
#include <gigatron/console.h>
#include <conio.h>
#include <stdarg.h>

#include "threads.h"

#pragma glcc option("PRINTF_SIMPLE")

// ----------------------------------------
// thread safe conio

GT_MUTEX(console_mutex);

void safe_cputs(int x, int y, int clr, const char *s)
{
	gt_mutex_lock(&console_mutex);
	textcolor(clr);
	cputsxy(x, y, s);
	gt_mutex_unlock(&console_mutex);
}

void safe_cprintf(int x, int y, int clr, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	gt_mutex_lock(&console_mutex);
	textcolor(clr);
	gotoxy(x,y);
	vcprintf(fmt, ap);
	gt_mutex_unlock(&console_mutex);
}


// ----------------------------------------
// prime numbers thread

GT_THREAD(th1);
GT_STACK(th1s, 128);

void run_primes(void *arg)
{
	unsigned int p = 1;
	int c = 1;
	safe_cputs(1, 4, YELLOW, "Prime");
	while (p < 65535u) {
		unsigned int d, q;
		p = p + 2;
		for (d = 3; (q = d * d) <= p; d += 2)
			if (p % d == 0)
				break;
		if (q > p) {
			static char b[8];
			safe_cputs(7, 4, YELLOW, utoa(p, b, 10));
			safe_cputs(20, 4, LIGHTGRAY, itoa(++c, b, 10));
		}
	}
}


// ----------------------------------------
// maze thread

GT_THREAD(th2);
GT_STACK(th2s, 128);

void draw(char *addr, int dir)
{
	int x = 1;
	if (dir < 0)
		addr += 7;
	while ((char)x) {
		SYS_VDrawBits(0x3f20, x, addr);
		addr += dir;
		x += x;
	}
}

void run_maze(void *arg)
{
	memset((char*)0x5f00, 0x3f, 160);
	for(;;)  {
		int rx;
		int r = SYS_Random();
		char *addr = (char*)((r & 0x78 | 0x60) << 8);
		do { rx = SYS_Random() & 0xf8; } while (rx >= 160);
		draw(addr + rx, (r & 1) ? +1 : -1);
	}
}


// ----------------------------------------
// main thread

void run_elapsed(void *arg)
{
	while (gt_running(&th1)) {
		long x = gt_clock();
		ldiv_t d = ldiv(x / 60, 60);
		safe_cprintf(1, 2, CYAN, "Elapsed %02d:%02d", (int)d.quot, (int)d.rem);
		gt_wait(x + 60);
	}
}

int main(void)
{
	gt_start(&th1, th1s, sizeof(th1s), run_primes, 0);
	gt_start(&th2, th2s, sizeof(th1s), run_maze, 0);
	run_elapsed(0);
	safe_cputs(1, 9, LIGHTMAGENTA, "Finished!");
	return 0;
}
