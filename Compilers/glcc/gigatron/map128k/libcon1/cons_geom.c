#include <stdlib.h>
#include <string.h>
#include <gigatron/console.h>
#include <gigatron/libc.h>
#include <gigatron/sys.h>

/* Providing a new version of this file is all
   that is needed to change the screen geometry. */

const struct console_info_s console_info = { 15, 26,
					     {  0,   16,  32,  48,  64,
						80,  96,  112, 128, 144,
						160, 176, 192, 208, 224  } };

extern void _con128ksetup(void);

void _console_reset(int fgbg)
{
        void *addr;
	_con128ksetup();
	addr = (void*)(videoTable[0] << 8);
	if (fgbg >= 0)
		_console_clear(addr, fgbg, 120);
}

