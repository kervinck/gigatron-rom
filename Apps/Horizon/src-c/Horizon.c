
/*----------------------------------------------------------------------+
 |                                                                      |
 |      Horizon.c -- Interactive 3D demo in C                           |
 |                                                                      |
 +----------------------------------------------------------------------*/

// Standard includes
#include <stdio.h>
#include <gigatron/sys.h>

#define screenWidth 160
#define screenHeight 120
#define horizonY 32
#define maxPitch (256 - screenWidth) // Aim for max possible pitch at bottom
#define color1 0x3f
#define color2 0x03
#define maxV 32

int main(void)
{
  int y, x, i, j, v = 8;
  byte *p, *q = &videoTable[240];

#define makeEven(v) (((v) + 1) & ~1) // Odd pitches give jagged edges?
#define calcPitch(y) makeEven((y) + (maxPitch - screenHeight + 1))

  // Draw tiles in perspective in screen memory
  for (y=screenHeight-1; y>=horizonY; y--) {
    int pitch = calcPitch(y);
    int width = screenWidth + pitch;
    i = (screenWidth/2) % pitch;
    j = (400 / (y-horizonY+40)) & 1 ? 0 : color1^color2; // Swap colors
    for (x=0; x<width; x++) {
      if (i-- == 0)
        i = pitch - 1;
      screenMemory[y][x] = (i+i >= pitch) ? color1^j : color2^j;
    }
    videoTop_v5 = y+y;      // Safe no-op on ROMs without videoTop
  }
  videoTop_v5 = 0;          // Show all

  // Interactive animation
  for (x=0; ; x+=v) {
    i = x & 255;                // Phase i

    // Calculate horizontal shift j at horizon
    for (y=0,j=0; y<calcPitch(horizonY); y++)
      j += i;

    // Quickly update videoTable, in steps of 2 pixel lines for more speed
    p = &videoTable[horizonY*2+1];
    i += i;

    // Synchronize with vertical blank
    { byte nextFrame = frameCount + 1;
      while (frameCount != nextFrame) /**/ ; }
    
    for (; p<q; p+=4) {
      *p = j >> 8;              // LCC nicely emits LD vAC+1 for the shift
      j = i + (j & 255);        // Faster operand order for LCC...
    }

    // Change speed by game controller or keys
    switch (buttonState) {
      case buttonRight: if (++v >  maxV) v =  maxV; break;
      case buttonLeft:  if (--v < -maxV) v = -maxV; break;
      case buttonA: case buttonB:        v =     0; break;
    }
  }
}

