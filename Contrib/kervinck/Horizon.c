
/*----------------------------------------------------------------------+
 |                                                                      |
 |      Horizon.c -- 3D demo in C                                       |
 |                                                                      |
 +----------------------------------------------------------------------*/

// Standard includes
#include <limits.h>
#include <stdio.h>

// System include
#include <Gigatron.h>

/*----------------------------------------------------------------------+
 |      Declarations                                                    |
 +----------------------------------------------------------------------*/

void setPhase(int phase);

/*----------------------------------------------------------------------+
 |      Main                                                            |
 +----------------------------------------------------------------------*/

char title[] = "C on Gigatron TTL";

#define horizonY 32

#define screenWidth 160
#define screenHeight 120

int main(void)
{
  int y, x, i, j;
  byte *p;

  /*
   *  Compiled C code is still too slow for mode 1
   */
  if (romType >= romTypeValue_ROMv2)
    SetMode_v2(2);

  // Aim for max pitch of 96 at line 119 (last)
  #define makeEven(v) (((v) + 1) & ~1)
  #define calcPitch(y) makeEven((y) + 256 - screenWidth - 119)

  /*
   *  Draw lanes in perspective in screen memory
   */
  for (y=screenHeight-1; y>=horizonY; y--) {
    int pitch = calcPitch(y);
    int width = screenWidth + pitch;
    i = (screenWidth/2) % pitch;
    for (x=0; x<width; x++) {
      if (i-- == 0)
        i = pitch - 1;
      screenMemory[y][x] = (i+i >= pitch) ? White : LightRed;
    }
    videoTop_DEVROM = y+y;      // Safe no-op on ROMs without videoTop
  }

  /*
   *  Write title centered in the sky
   */
  y = horizonY - 8;
  x = screenWidth - (sizeof title - 1) * 6;
  ScreenPos = (int) &screenMemory[y/2][x/2];
  puts(title);
  videoTop_DEVROM = 0;          // Show all

  /*
   *  Animate
   */
  for (x=0; ; x+=10) {
    i = x & 255;                // Phase

    // Calculate horizontal shift for horizon
    for (y=0 ; y<calcPitch(horizonY); y++)
      j += i;

    // Synchronize with vertical blank
    BusyWait(1);

    // Quickly update videoTable, in steps of 2 for speed
    i += i;
    for (y=horizonY*2+1; y<screenHeight*2; y+=4) {
      videoTable[y] = j >> 8;
      j = (j & 255) + i;
    }
  }
}

/*----------------------------------------------------------------------+
 |                                                                      |
 +----------------------------------------------------------------------*/

