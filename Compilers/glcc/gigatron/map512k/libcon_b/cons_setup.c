#include <stdlib.h>
#include <string.h>
#include <gigatron/console.h>
#include <gigatron/libc.h>
#include <gigatron/sys.h>

#if _GLCC_VER < 104010
# error "This library requires GLCC > 1.4-10"
#endif

static void console_exitm_msgfunc(int retcode, const char *s)
{
  if (s) {
    static struct console_state_s rst = {3, 0, 0, 1, 1};
    console_state = rst;
    console_state.cy = console_info.nlines;
    console_print(s, console_info.ncolumns);
  }

  {
    /* Halting code (flash pixel using the proper screen) */
    char c = 0;
    char *row = (char*)(((*(char*)0x100)|0x80) << 8);
    SYS_ExpanderControl(0xe0f0u);
    SYS_ExpanderControl(ctrlBits_v5 & 0x3f);
    while (1)
      row[(char)retcode] = ++c;
  }
}

void _console_setup(void)
{
  /* TODO: Test availability of the 512KB extension */

  // Display from pages 14/15. 
  SYS_ExpanderControl(0x0fe0u);
  _exitm_msgfunc = console_exitm_msgfunc;
  console_clear_screen();
}
