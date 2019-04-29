
/*----------------------------------------------------------------------+
 |                                                                      |
 |      DemoLib.c -- Demonstrate ideas for standard library             |
 |                                                                      |
 +----------------------------------------------------------------------*/

#include <Gigatron.h>
#include <stdio.h>

/*----------------------------------------------------------------------+
 |      main                                                            |
 +----------------------------------------------------------------------*/

int main(void)
{
  ClearScreen();
  puts("Hello Gigatron! How are you today?");

  while (1) {
    int p = ScreenPos;
    int c;
    putchar(127);               // Cursor symbol
    ScreenPos = p;              // Go back

    c = WaitKey();

    if (c == '\n') {
      putchar(' ');
      ScreenPos = p;
    }

/*
    switch (c) {
    case buttonLeft:
      ScreenPos -= 1;
      break;
    case buttonRight:
      ScreenPos += 1;
      break;
    case buttonUp:
      ScreenPos -= 0x100;
      break;
    case buttonDown:
      ScreenPos += 0x100;
      break;
    }
*/

    putchar(c);
  }
  return 0;
}

/*----------------------------------------------------------------------+
 |                                                                      |
 +----------------------------------------------------------------------*/

