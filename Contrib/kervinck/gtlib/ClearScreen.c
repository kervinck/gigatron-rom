
/*----------------------------------------------------------------------+
 |                                                                      |
 |      ClearScreen -- Clear standard screen memory                     |
 |                                                                      |
 +----------------------------------------------------------------------*/

#include "Gigatron.h"

void ClearScreen(byte color)
{
  int p = (int)screenMemory;

  sysFn = SYS_VDrawBits_134;    // SYS function plots 8 pixels vertically
  sysArgs[0] = color;           // Background color
  sysArgs[2] = 0;               // All-zero bit pattern: only background

  do {
    do {
      *(int*)(sysArgs+4) = p;   // Screen address
      __syscall(203);           // == 270-134/2
      p += 0x800;               // Step 8 pixels down
    } while (p >= 0);

    p += 0x8801;                // Step 120 pixels up, 1 pixel right
  } while ((p & 255) != 160);   // Until reaching x position 160
}

/*----------------------------------------------------------------------+
 |                                                                      |
 +----------------------------------------------------------------------*/

