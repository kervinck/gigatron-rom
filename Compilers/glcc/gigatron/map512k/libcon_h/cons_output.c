#include <stdlib.h>
#include <string.h>
#include <gigatron/console.h>
#include <gigatron/libc.h>
#include <gigatron/sys.h>

struct console_state_s console_state = { CONSOLE_DEFAULT_FGBG, 0, 0, 1, 1 };

static char *cons_addr(void)
{
	register int x, y;
	if (((x = console_state.cx) >= 0) &&
	    (console_info.ncolumns - x > 0) &&
	    ((y = console_state.cy) >= 0) &&
	    (console_info.nlines - y > 0) )
		return (char*)((videoTable[console_info.offset[y]] << 8) + x * 3);
	return 0;
}

void console_clear_screen(void)
{
	_console_reset(console_state.fgbg);
	console_state.cx = console_state.cy = 0;
}

void console_clear_to_eol(void)
{
	register char *addr;
	if (addr = cons_addr())
		_console_clear(addr, console_state.fgbg, 8);
}

static void scroll(int nl, char pg0)
{
	register int i, j;
	register char pg, *vt;
	i = 0;
	while (i != nl) {
		vt = &videoTable[console_info.offset[i]];
		if (++i == nl)
			pg = pg0;
		else
			pg = videoTable[console_info.offset[i]];
		for (j = 0; j != 4; j++, vt += 2, pg += 2)
			*vt = pg;
	}
}

int console_print(register const char *s, register int len)
{
	register int nc = 0;
	register int *pcx = &console_state.cx;
	register int *pcy = &console_state.cy;
	while (nc < len && s[nc]) {
		register int n;
		register char *addr;
		n = console_info.ncolumns;
		if (console_state.wrapx && *pcx - n >= 0) {
			*pcx = 0;
			*pcy += 1;
		}
		n = console_info.nlines;
		if (console_state.wrapy && *pcy - n >= 0) {
			int pg0 = videoTable[console_info.offset[0]];
			_console_clear((char*)(pg0 << 8), console_state.fgbg, 8);
			scroll(n, pg0);
			*pcy = n - 1;
		}
		if ((! _console_ctrl(s[nc])) && (addr = cons_addr()) &&
		    (n = _console_printchars(console_state.fgbg, addr, s + nc, len - nc)) ) {
			*pcx += n;
			nc += n;
		} else {
			nc += 1;
		}
	}
	return nc;
}

DECLARE_INIT_FUNCTION(_console_setup);

