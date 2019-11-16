
/*----------------------------------------------------------------------+
 |                                                                      |
 |      Horizon.c -- Interactive 3D demo in C                           |
 |                                                                      |
 +----------------------------------------------------------------------*/

// Standard includes
#include <stdio.h>

// System include
#include <Gigatron.h>

/*----------------------------------------------------------------------+
 |      Definitions                                                     |
 +----------------------------------------------------------------------*/

char title[] = "C on Gigatron TTL";
#define screenWidth 160
#define screenHeight 120
#define horizonY 32
#define maxPitch (256 - screenWidth) // Aim for max possible pitch at bottom
#define color1 White
#define color2 LightRed
#define maxV 64

/*----------------------------------------------------------------------+
 |      Main                                                            |
 +----------------------------------------------------------------------*/

int main(void)
{
  int y, x, i, j, v = 8;

  // Compiled C code from LCC is still too slow for mode 1
  if (romType >= romTypeValue_ROMv2)
    SetMode_v2(2);

  #define makeEven(v) (((v) + 1) & ~1) // Odd pitches give jagged edges?
  #define calcPitch(y) makeEven((y) + maxPitch - screenHeight + 1)

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
    videoTop_DEVROM = y+y; // Safe no-op on ROMs without videoTop
  }

  // Write title centered in the sky
  y = horizonY - 8;
  x = screenWidth - (sizeof title - 1) * 6;
  ScreenPos = (int) &screenMemory[y/2][x/2];
  puts(title);
  videoTop_DEVROM = 0; // Show all

  // Interactive animation
  for (x=0; ; x+=v) {
    i = x & 255; // Phase i

    // Calculate horizontal shift j at horizon
    for (y=0,j=0; y<calcPitch(horizonY); y++)
      j += i;

    // Quickly update videoTable, in steps of 2 pixel lines for more speed
    y = horizonY*2+1;
    i += i;
    BusyWait(1); // Synchronize with vertical blank
    for (; y<screenHeight*2; y+=4) {
      videoTable[y] = j >> 8;   // LCC nicely emits LD vAC+1 for the shift
      j = i + (j & 255);        // Fasted operand order for LCC...
    }

    // Change speed by game controller or keys
    switch (buttonState) {
      case buttonRight: if (++v >  maxV) v =  maxV; break;
      case buttonLeft:  if (--v < -maxV) v = -maxV; break;
      case buttonA: case buttonB:        v =     0; break;
    }
  }
}

/*----------------------------------------------------------------------+
 |                                                                      |
 +----------------------------------------------------------------------*/
