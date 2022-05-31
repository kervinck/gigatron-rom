#include <stdlib.h>
#include <string.h>
#include <gigatron/console.h>
#include <gigatron/libc.h>
#include <gigatron/sys.h>

void console_clear_line(int y)
{
	if (y >= 0 && y < console_info.nlines) {
		int pg = videoTable[console_info.offset[y]];
		_console_clear((char*)(pg<<8), console_state.fgbg, 8);
	}
}

void console_scroll(int y1, int y2, int n)
{
	char buf[15];
	register int i, j, k;
	register int d = y2 - y1;
	register char *vt;
	if (d - 1 > 0) {
		for(i = y1; i!= y2; i++)
			buf[i] = videoTable[console_info.offset[i]];
		for(i = y1; i!= y2; i++) {
			j = i + n;
			while (j < y1)
				j += d;
			while (j >= y2)
				j -= d;
			j = buf[j];
			vt = &videoTable[console_info.offset[i]];
			for (k=0; k!=8; k++, vt+=2, j+=1)
				*vt = j;
		}
	}
}
