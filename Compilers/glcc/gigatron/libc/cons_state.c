#include <stdlib.h>
#include <string.h>
#include <gigatron/console.h>
#include <gigatron/libc.h>
#include <gigatron/sys.h>

/* console state */
__near struct console_state_s console_state = { CONSOLE_DEFAULT_FGBG, 0, 0, 1, 1 };

/* cause setup code to be called */
DECLARE_INIT_FUNCTION(_console_setup);

