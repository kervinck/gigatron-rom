
#include <Gigatron.h>

void Newline(void)
{
  // Go down 1..8 pixels, realigning at next 8 pixel rows
  // for easy detection of reaching bottom of screen
  ScreenPos = (ScreenPos & 0x7800) + 0x0800;

  if (ScreenPos < 0)            // Wrap around screen memory
    ScreenPos = (int)screenMemory;

  // At the top of the visible screen
  if ((ScreenPos >> 8) == videoTable[0]) {
    int i;

    // Clear this line
    sysFn = SYS_VDrawBits_134;  // SYS function plots 8 pixels vertically
    sysArgs[0] = BgColor;       // Set background color
    sysArgs[2] = 0;             // Set bit pattern: only background
    *(int*)(sysArgs+4) = ScreenPos;// Set screen address
    do {
      __syscall(134);
      sysArgs[4]++;             // Step 1 pixel right
    } while (sysArgs[4] != 160);// Until reaching X position 160

    // Scroll up by manipulating video indirection table
    i = 240;
    do {
      byte page;
      i -= 2;
      page = videoTable[i] + 8;
      videoTable[i] = page & 128 ? page - 120 : page;
    } while (i);
  }

  ScreenPos += Indent;          // Apply small indentation
}

