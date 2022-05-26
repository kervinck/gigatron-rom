#include <stdlib.h>
#include <string.h>
#include <gigatron/console.h>
#include <gigatron/libc.h>
#include <gigatron/sys.h>


int console_getkey(void)
{
	static char last = 0xff;
	register char ch = serialRaw;
	if (last != ch) {
		last = ch;
		if (ch != 255)
			return ch;
	}
	return -1;
}

static void update_cursor(int on)
{
	console_print(on ? "\x7f\b" : "\x20\b", 2);
}

int console_waitkey()
{
	register int btn;
	register int ofc = frameCount & 0xff;
	for(;;) {
		if ((btn = console_getkey()) >= 0)
			break;
		if ((frameCount ^ ofc) & 0xf0) {
			update_cursor(ofc & 0x10);
			ofc = frameCount;
		}
	}
	update_cursor(0);
	return btn;
}

