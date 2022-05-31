#include <stdlib.h>
#include <string.h>
#include <gigatron/libc.h>
#include <gigatron/sys.h>

#define console_info_s console_info_s_dummy
#define console_info console_info_dummy
#include <gigatron/console.h>
#undef console_info_s
#undef console_info

const struct console_info_s {
  int nlines;		     /* number of lines   */
  int ncolumns;                /* number of columns */
  unsigned char offset[30];    /* offset of each line in the video table */
} console_info = { 30, 52,
                   {  0,   8,   16,  24,  32,  40,  48,  56,  64,  72,
                      80,  88,  96,  104, 112, 120, 128, 136, 144, 152,
                      160, 168, 176, 184, 192, 200, 208, 216, 224, 232  } };


void _console_reset(int fgbg)
{
	int i;
	int *table = (int*)videoTable;

        // Setup display mode
        SYS_ExpanderControl(0x0ee0u);
        *(char*)0xb |= 1;
        
        // Clear screen
	if (fgbg >= 0)
          {
            _console_clear((char*)0x0800u, fgbg, 120);
            _console_clear((char*)0x8000u, fgbg, 120);
          }

        // Reset video table (one entry for two lines)

        //// Linear addresses
        //// - Even pixels of row Y at location 0x70800 + 0xYY00 + [0..159]
        //// - Odd pixels of row Y at  location 0x60800 + 0xYY00 + [0..159]
        //// Bank information
        //// - Banks 14 and 15 for even pixels
        //// - Banks 12 and 13 for odd pixels
	for (i=0x08; i != 0xf8; i += 2)
          *table++ = i;
}

