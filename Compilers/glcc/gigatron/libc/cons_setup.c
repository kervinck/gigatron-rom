#include <stdlib.h>
#include <string.h>
#include <gigatron/console.h>
#include <gigatron/libc.h>
#include <gigatron/sys.h>

static void console_exitm_msgfunc(int retcode, const char *s)
{
	if (s) {
		static struct console_state_s rst = {3, 0, 0, 1, 1};
		console_state = rst;
		console_state.cy = console_info.nlines;
		console_print(s, console_info.ncolumns);
	}
}

void _console_setup(void)
{
	console_clear_screen();
	_exitm_msgfunc = console_exitm_msgfunc;
}
