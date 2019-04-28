
#include <Gigatron.h>

void Newline(void)
{
  // TODO Scroll up screen when needed (using videoTable)

  // Go down 8..15 pixels, realigning at 8 pixel rows, and indent
  ScreenPos = ((ScreenPos + 0x0700) & 0xf800) + (0x0800 + Indent);

  if (ScreenPos < 0)
    ScreenPos = (int)screenMemory + Indent; // Wrap around
}

