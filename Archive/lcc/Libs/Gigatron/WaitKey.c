
#include <Gigatron.h>

/*
 *  Wait until we observe a key change to a non-idle state
 */
byte WaitKey(void)
{
  byte s, t = serialRaw;

  do {
    s = t;
    t = serialRaw;
  } while (s == t || t == 255);

  return t;
}

