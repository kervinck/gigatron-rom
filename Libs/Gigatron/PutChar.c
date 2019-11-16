#include <Gigatron.h>

void PutChar(int c)
{
  byte *bitmap;
  int i;

  // Accept newlines
  if (c == '\n') {
    Newline();
    return;
  }

  // Automatic line wrapping
  if (((byte*)&ScreenPos)[0] > 160-6)
    Newline();

  // Avoid writing outside screen memory
  // This also forces a newline if ScreenPos is still 0 after startup
  i = ((byte*)&ScreenPos)[1]; // ScreenPos high byte
  if (((i-8) & 255) > 120-8)
    Newline();

  // Ignore non-printable characters
  // XXX This still can print rubish?
  i = c - 32;
  if ((unsigned)i >= 128-32)
    return;

  // Select bitmap for character from built-in font
  if (i < 82-32)
    bitmap = font32up; // char 32..81
  else {
    i -= 82-32;
    bitmap = font82up; // char 82..127
  }
  bitmap = &bitmap[(i << 2) + i];

  // Prepare SYS call
  sysFn = SYS_VDrawBits_134;
  sysArgs[0] = BgColor;
  sysArgs[1] = Color;
  *(int*)(sysArgs+4) = ScreenPos;

  // Draw bitmap to screen as 5 vertical slices
  for (i=5; i>0; --i, bitmap++) {
    sysArgs[2] = __lookup(0, bitmap);
    __syscall(203);
    sysArgs[4]++;
  }

  // Clear 6th slice
  sysArgs[2] = 0;
  __syscall(203);

  // Advance position
  ScreenPos += 6;
}
