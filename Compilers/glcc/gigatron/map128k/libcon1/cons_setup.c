#include <stdlib.h>
#include <string.h>
#include <gigatron/console.h>
#include <gigatron/libc.h>
#include <gigatron/sys.h>

#if _GLCC_VER < 105061
# error "This library requires GLCC > 1.5-61"
#endif

static void console_exitm_msgfunc(int retcode, const char *s)
{
  if (s) {
	  console_state.fgbg = 3;
	  console_state_set_cycx(console_info.nlines);
	  console_state_set_wrap(0x101);
	  console_print(s, console_info.ncolumns);
  }
  {
	  /* Halting code (flash pixel using the proper screen) */
	  char c = 0;
	  char *row = (char*)(((*(char*)0x100)|0x80) << 8);
	  SYS_ExpanderControl(ctrlBits_v5 & 0x3f | 0x40);
	  while (1)
		  row[(char)retcode] = ++c;
  }
}

void _console_setup(void)
{
	_exitm_msgfunc = console_exitm_msgfunc;
	console_clear_screen();
}
