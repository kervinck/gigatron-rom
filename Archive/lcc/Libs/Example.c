
/*----------------------------------------------------------------------+
 |                                                                      |
 |      Example.c -- Demonstrate ideas for standard library             |
 |                                                                      |
 +----------------------------------------------------------------------*/

// Standard includes
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>

// System include
#include <Gigatron.h>

/*----------------------------------------------------------------------+
 |      main                                                            |
 +----------------------------------------------------------------------*/

int main(void)
{
  char *name = "Gigatron";
  char punct = '!';
  char text[64];

  snprintf(text, sizeof text, "Hello %s%c How are you today? %d%%?", name, punct, 100);
  puts(text);

  BusyWait(60);                 // Wait one second

  ClearScreen();

  // Demo printf and varargs
  printf("%d %d %u\n",          1972, -327, UINT_MAX);
  printf("%07d %07d %07u\n",    1972, -327, UINT_MAX);
  printf("%+7d %+7d %+7u\n",    1972, -327, UINT_MAX);
  printf("%+07d %+07d %+07u\n", 1972, -327, UINT_MAX);

  puts("Ready");

  BgColor = LightBlue;          // Slightly different for effect
  while (1) {
    int c;

    PutChar(127);               // Cursor symbol
    ScreenPos -= 6;             // Go back

    c = WaitKey();

    PutChar(' ');               // Remove cursor
    ScreenPos -= 6;             // Go back

    switch (buttonState) {      // Handle arrow keys
    case buttonLeft:  ScreenPos -=     1; break;
    case buttonRight: ScreenPos +=     1; break;
    case buttonUp:    ScreenPos -= 0x100; break;
    case buttonDown:  ScreenPos += 0x100; break;
    case 127:                   // Delete key (also the [A] button...)
      if ((ScreenPos & 255) >= 6)
        ScreenPos -= 6;
      break;
    default:
      PutChar(c);               // Put character on screen
      break;
    }
  }
  return 0;
}

/*----------------------------------------------------------------------+
 |                                                                      |
 +----------------------------------------------------------------------*/

