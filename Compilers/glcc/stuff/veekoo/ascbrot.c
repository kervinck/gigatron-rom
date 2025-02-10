/*----------------------------------------------------------------------+
 |                                                                      |
 |     ascbrot.c -- demonstrate fractal in text                         |
 |                                                                      |
 | Program `ascbrot.c` was originally written by veekoo.                |
 | Go to https://github.com/veekooFIN/GigatronTTL-Fractals for more.    |
 |                                                                      |
 +----------------------------------------------------------------------*/
 
// Standard includes
#include <conio.h>

#ifndef TIMER
# define TIMER 1
#endif
#if TIMER
# include <gigatron/libc.h>
#endif

#define HEIGHT 15
#define WIDTH 26
#define SCALE 1.6
#define YSTEP 1
#define XSTEP 1

int mandelbrot(float x, float y) {
 float zz;
 float a;
 float b;
 float a2;
 float b2;
 float atemp;
 int i;
  
  a = 0;
  b = 0;  
  i = 48;
  while (i < 58)
  {
    a2 = a * a;
    b2 = b * b;
    zz = a2 + b2;
    if(zz > 4) break;
    
    atemp = a2 - b2 + x;
    b = 2.0 * a * b + y;
    a = atemp;
    i++;
  }
  return i;
}

void main(void) {
  int x, y, data;
  float sx, sy;
   
#if TIMER
  unsigned int ticks = _clock();
#endif

  for(y = 0; y < HEIGHT; y = y + YSTEP ) {
    for(x = 0; x < WIDTH; x = x + XSTEP ) {
      sx = -0.7 + (SCALE * (WIDTH/2.0 - x) / (WIDTH/2.0))*(-1);
      sy = (SCALE * (HEIGHT/2.0 - y) / (HEIGHT/2.0))*(-0.75);
      data = mandelbrot(sx, sy);
      gotoxy(x+1, y+1);
      textcolor(((data-48)*6+1) & 0x3f);
      putch(data);
    }
  }
#if TIMER
  ticks = _clock() - ticks;
  gotoxy(8,8);
  textcolor(0);
  textbackground(0x3f);
  mincprintf(" %d %d/60 seconds ", ticks/60, ticks % 60);
#endif
}
