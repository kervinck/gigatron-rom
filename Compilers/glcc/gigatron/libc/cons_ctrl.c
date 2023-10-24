#include <stdlib.h>
#include <string.h>
#include <gigatron/console.h>
#include <gigatron/libc.h>
#include <gigatron/sys.h>

/* defined in cons_asm.s */
extern char *_console_addr();

/* defined in conio.h */
extern void clrscr();
extern void clreol();

/* Handle control characters other than BS, CR, LF */
int _console_ctrl(int c)
{
	register char *addr;
	switch (c) {
	case '\t':  /* TAB */
		console_state.cx = (console_state.cx | 3) + 1;
		break;
	case '\a': /* BELL */
		_console_bell(4);
		break;
	case '\f': /* FF */
		clrscr();
		break;
	case '\v': /* VT */
		clreol();
		break;
	default:
		return 0;
	}
	return 1;
}

