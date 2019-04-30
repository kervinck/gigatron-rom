
/*----------------------------------------------------------------------+
 |                                                                      |
 |      Example.c -- Demonstrate ideas for standard library             |
 |                                                                      |
 +----------------------------------------------------------------------*/

#include <Gigatron.h>
#include <stdio.h>

/*----------------------------------------------------------------------+
 |      main                                                            |
 +----------------------------------------------------------------------*/

int main(void)
{
  puts("Hello Gigatron! How are you today?");
  //Wait(60);

  ClearScreen();

  puts("Ready");

  BgColor = LightBlue;

  while (1) {
    int c;

    putchar(127);               // Cursor symbol
    ScreenPos -= 6;             // Go back

    c = WaitKey();

    putchar(' ');               // Remove cursor
    ScreenPos -= 6;             // Go back

    switch (c) {                // Handle arrow keys and/or buttons
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
    default:
      putchar(c);               // Put character on screen
      break;
    }
  }
  return 0;
}

/*----------------------------------------------------------------------+
 |                                                                      |
 +----------------------------------------------------------------------*/

