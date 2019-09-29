
#include <Gigatron.h>

/*
 *  Spin for `frames` vertical blanks of the Gigatron VGA signal (59.98 Hz)
 *  Good for waits of up to 9 minutes. Negative delays return immediately.
 */
void BusyWait(int frames)
{
  byte nextFrame = frameCount;
  while (frames-- > 0) {
    nextFrame++;
    while (frameCount != nextFrame)
      ;
  }
}

