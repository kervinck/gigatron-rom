
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
    putchar(c);
  }
  return 0;
}

/*----------------------------------------------------------------------+
 |                                                                      |
 +----------------------------------------------------------------------*/

