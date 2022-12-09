#include <stdlib.h>
#include <string.h>
#include <gigatron/console.h>
#include <gigatron/libc.h>
#include <gigatron/sys.h>

#define CTRL_TABS 1
#define CTRL_BELL 1

int _console_ctrl(register int c)
{
	switch(c) {
	case '\b': /* backspace */
		if (console_state.cx > 0)
			console_state.cx -= 1;
		else if (console_state.cy > 0) {
			console_state.cx = console_info.ncolumns-1;
			console_state.cy -= 1;
		}
		break;
	case '\n': /* lf */
		console_state.cy += 1;
	case '\r': /* cr */
		console_state.cx = 0;
		break;
#if CTRL_TABS
	case '\t':  /* tab */
		console_state.cx = (console_state.cx | 3) + 1;
		break;
	case '\f':
		console_clear_screen();
		break;
	case '\v':
		console_clear_to_eol();
		break;
#endif
#if CTRL_BELL
	case '\a':
		_console_bell(4);
		break;
#endif
	default:
		return 0;
	}
	return 1;
}

