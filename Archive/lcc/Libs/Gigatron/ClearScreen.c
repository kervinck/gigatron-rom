
/*----------------------------------------------------------------------+
 |                                                                      |
 |      ClearScreen -- Clear standard screen memory                     |
 |                                                                      |
 +----------------------------------------------------------------------*/

#include <Gigatron.h>

extern int ScreenPos            = 0;
extern byte Color               = White;
extern byte BgColor             = Blue;

void ClearScreen(void)
{
  int p = (int)screenMemory;
  ScreenPos = p + Indent;       // Go back to top left of screen

  sysFn = SYS_VDrawBits_134;    // SYS function plots 8 pixels vertically
  sysArgs[0] = BgColor;         // Set background color
  sysArgs[2] = 0;               // Set bit pattern: only background

  do {
    do {
      *(int*)(sysArgs+4) = p;   // Set screen address
      __syscall(134);
      p += 0x800;               // Step 8 pixels down
    } while (p >= 0);

    p += -120*256 + 1;          // Step 120 pixels up, 1 pixel right

  } while ((p & 255) != 160);   // Until reaching X position 160

  for (p=120-1; p>=0; p--)      // Reset video indirection table
    videoTable[p+p] = ((int)screenMemory >> 8) + p;
}

/*----------------------------------------------------------------------+
 |                                                                      |
 +----------------------------------------------------------------------*/

