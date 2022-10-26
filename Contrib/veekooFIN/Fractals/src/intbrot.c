/*----------------------------------------------------------------------+
 |                                                                      |
 |     intbrot.c -- demonstrate fractal in gfx / quick and dirty        |
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

#define WIDTH 40
#define HEIGHT 30
#define NORM_BITS 12
#define F 4096

int mandelbrot(int real0, int imag0) {
  int realq, imagq; 
  int real, imag;
  int i;

  real = real0;
  imag = imag0;
  for (i = 0; i < 15; i++)
  {
    realq = (real * real) >> NORM_BITS;
    imagq = (imag * imag) >> NORM_BITS;

    if ((realq + imagq) > 16384) break;

    imag = ((real * imag) >> (NORM_BITS - 1)) + imag0;
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
  int realmin, imagmin, realmax, imagmax;
  int deltareal, deltaimag, real0, imag0;  
  
  col[0] = 0x01;
  col[1] = 0x02;
  col[2] = 0x03;
  col[3] = 0x07;
  col[4] = 0x0b;
  col[5] = 0x0f;
  col[6] = 0x0e;
  col[7] = 0x0d;
  col[8] = 0x0c;
  col[9] = 0x3c;
  col[10] = 0x38;
  col[11] = 0x34;
  col[12] = 0x30;
  col[13] = 0x20;
  col[14] = 0x10;
  col[15] = 0x00;
  
  realmin = (int) ((-2.0) * (float) F);
  realmax = (int) ((0.7) * (float) F);
  imagmin = (int) ((-1.2) * (float) F);
  imagmax = (int) ((1.2) * (float) F); 
   
  for(y = 0; y < HEIGHT; y++ ) {
    for(x = 0; x < WIDTH; x++ ) {
      drawPixel(x,y,col[15]);
    }
  } 
  
  deltareal = (realmax - realmin) / (int) WIDTH;
  deltaimag = (imagmax - imagmin) / (int) HEIGHT;

  real0 = realmin; 
  for(x = 0; x < WIDTH; x++ ) {
    imag0 = imagmax;
    for(y = 0; y < HEIGHT; y++ ) {
      data = mandelbrot(real0, imag0);
      drawPixel(x,y,col[data]);
      imag0 -= deltaimag;
    }
    real0 += deltareal;
  }
}
