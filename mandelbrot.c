
/*
   Mandelbrot study using just 16-bit arithmetic, no multiply

   2018-01-21 (marcelk) Study for Gigatron version
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define BITS 7
#define ONE (1<<BITS)

// Calculate (A*B) >> BITS
static
short mulShift(short A, short B)
{
  // Extract sign and absolute values
  int sign = 0;
  if (A < 0) {
    A = -A;
    sign = 1;
  }
  assert(A >= 0);
  assert(A < 0x800);
  if (B < 0) {
    B = -B;
    sign ^= 1;
  }
  assert(B >= 0);

  short REF = (A * B) >> BITS;

  // Multiply
  short shift = BITS;
  int C = 0;
  for (short bit=0x400; bit; bit>>=1) {
    assert(C < 0xc000);
    if (C < 0x4000)
      C += C;
    else { //Shift prematurely in an attempt to avoid overflow
      B >>= 1;
      shift--;
    }
    // Add partial product
    if (A - bit >= 0) {
      A -= bit;
      C += B;
    }
  }
  assert(C < 0x10000);

  // Shift
  assert(shift > 0);
  do
    C >>= 1;
  while (--shift > 0);

  // The result can be off by one
  assert(abs(C - REF) <= 1);

  // Apply sign to return value
  return sign ? -C : C;
}

int main(void)
{
  for (short Y0=-180; Y0<180; Y0+=3) {
    for (short X0=-320; X0<160; X0+=3) {
      // First check if we are inside one of the main bulbs for
      // a quick bailout (Wikipedia)
      // (x+1)^ + y^2 < 1/16
      if (mulShift(X0+ONE, X0+ONE) + mulShift(Y0, Y0) < ONE/16) {
        putchar('~');
        continue;
      }
      // q*(q + x - 1/4) < 1/4*y^2, where q = (x - 1/4)^2 + y^2
      int Q = mulShift(X0-ONE/4, X0-ONE/4) + mulShift(Y0, Y0);
      if (mulShift(Q, Q + X0 - ONE/4)*4 < mulShift(Y0, Y0)) {
        putchar('~');
        continue;
      }

      // Otherwise run the escape algorithm
      int X=0, XX=0, Y=0, YY=0;
      int i;
      for (i=1; i<64; i++) {
        assert(abs(X) <= (2<<BITS));
        assert(abs(Y) <= (2<<BITS));

        // Mandelbrot function: z' := z^2 + c
        Y = mulShift(X, 2*Y) + Y0;
        X = XX - YY + X0;

        assert(abs(XX) <= (4<<BITS));
        assert(abs(YY) <= (4<<BITS));

        // Calculate squares
        XX = mulShift(X, X);
        YY = mulShift(Y, Y);

        if (XX + YY > 4<<BITS)
          break;
      }
      putchar(' '+ (i&63));
    }
    putchar('\n');
  }
  return 0;
}

