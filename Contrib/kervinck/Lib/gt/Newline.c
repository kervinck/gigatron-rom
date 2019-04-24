
#include <Gigatron.h>

void Newline(void)
{
   // Go down 8..15 pixels, align at 8 pixel rows. Indent 2 pixels
   _ScreenPos = ((_ScreenPos + 0x0700) & 0xf800) + 0x800 + _Indent;

   // TODO: _ScreenPos.x = _Indent;

   if (_ScreenPos < 0)
     _ScreenPos = (int)screenMemory + _Indent;
   // TODO Scroll up screen when needed using videoTable
}

