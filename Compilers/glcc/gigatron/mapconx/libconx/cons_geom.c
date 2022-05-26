#include <stdlib.h>
#include <string.h>
#include <gigatron/console.h>
#include <gigatron/sys.h>


/* This file contains the functions that need to change when one
   changes the screen geometry by playing with the video table. */

#define NLINES  10
#define BLANKMEM  0x2f
#define CHARSMEM  0x30

const struct console_info_s console_info = { 10, 26,
					     {  4,   28,  52,  76,  100,
						124, 148, 172, 196, 220  } };

void _console_reset(int fgbg)
{
	int i, j;
	int *table = (int*)videoTable;
	unsigned int page = CHARSMEM;
	if (fgbg >= 0)
		_console_clear((char*)(BLANKMEM << 8), fgbg, 81);
	for (i = 0; i != NLINES; i++)
		for (j = 0; j != 12; j++, table++)
			if (j - 2 < 0 || j - 10 >= 0)
				*table = BLANKMEM;
			else
				*table = page++;
}

