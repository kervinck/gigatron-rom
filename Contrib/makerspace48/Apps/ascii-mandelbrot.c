
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




int mandelbrot(void)
{
  int z;
  int x;
  int y;
  int x0;
  int y0;
  int x2;
  int y2;
  int iteration;
  int xtemp;

z = 0;
x = 0;
y0 = 0;
x0 = 0;
y0 = 0;
  for (y0 = -7; y0 < 8; y0 = y0 + 1) 
  {
    for (x0 = -17; x0 < 9; x0 = x0 + 1) 
    {

      x = 0;
      y = 0;
      iteration = 47;
      while (iteration < 58)
      {
        x2 = x*x/8;
        y2 = y*y/8;
        z = x2 + y2;
        if (z>32) break;

        xtemp = x2 - y2 + x0;
        y = x*y/4 + y0;
        x = xtemp;
        iteration = iteration + 1;

      }
      if(x0+y0 < 16) putchar(iteration);
    }
  }
  return 0;
}

int main(void)
{
printf("     ASCII Mandelbrot\n");
printf("     On Gigatron in c\n");
printf("       Makerspace48\n");
printf("       Arendal 2019\n\n\n\n\n\n\n\n");
mandelbrot();
  return 0;
}
