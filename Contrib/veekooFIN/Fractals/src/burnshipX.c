/*----------------------------------------------------------------------+
 |                                                                      |
 |     burnshipX.c -- demonstrate fractal in gfx / quick and dirty      |
 |                                                                      |
 +----------------------------------------------------------------------*/
 
// Standard includes
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <gigatron/console.h>
#include <gigatron/sys.h>

#define WIDTH 160
#define HEIGHT 120
#define NORM_BITS 13
#define F 8192

int burnship(long real0, long imag0) {
  long realq, imagq; 
  long real, imag;
  int i;

  real = real0;
  imag = imag0;
  for (i = 0; i < 15; i++)
  {
    realq = (real * real) >> NORM_BITS;
    imagq = (imag * imag) >> NORM_BITS;

    if ((realq + imagq) > 32768) break;

    imag = abs((real * imag) >> (NORM_BITS - 1)) + imag0;
    real = realq - imagq + real0;
  }
  return i;
}

void drawPixel(int x, int y, int color)
{
  screenMemory[y][x] = color;
}

void main(void) {
  int x, y, data;
  int col[16];
  long realmin, imagmin, realmax, imagmax;
  long deltareal, deltaimag, real0, imag0;  
  
  SYS_SetMode(3);
  
  col[14] = 0x01;
  col[13] = 0x02;
  col[12] = 0x03;
  col[11] = 0x07;
  col[10] = 0x0b;
  col[9] = 0x0f;
  col[8] = 0x0e;
  col[7] = 0x0d;
  col[6] = 0x0c;
  col[5] = 0x3c;
  col[4] = 0x38;
  col[3] = 0x34;
  col[2] = 0x30;
  col[1] = 0x20;
  col[0] = 0x10;
  col[15] = 0x00;
  
  realmin = (long) ((-1.8) * (float) F);
  realmax = (long) ((-1.7) * (float) F);
  imagmin = (long) ((-0.08) * (float) F);
  imagmax = (long) ((0.01) * (float) F); 
   
  for(y = 0; y < HEIGHT; y++ ) {
    for(x = 0; x < WIDTH; x++ ) {
      drawPixel(x,y,col[15]);
    }
  } 
  
  deltareal = (realmax - realmin) / (long) WIDTH;
  deltaimag = (imagmax - imagmin) / (long) HEIGHT;

  real0 = realmin; 
  for(x = 0; x < WIDTH; x++ ) {
    imag0 = imagmin;
    for(y = 0; y < HEIGHT; y++ ) {
      data = burnship(real0, imag0);
      drawPixel(x,y,col[data]);
      imag0 += deltaimag;
    }
    real0 += deltareal;
  }
  SYS_SetMode(0);
}
