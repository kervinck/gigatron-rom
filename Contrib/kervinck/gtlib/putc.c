
#include "Gigatron.h"
#include "stdio.h"

int putc(int c)
{
        unsigned char* bitmap;
        unsigned i;

        i = c - 32;
        if (i < 50) {
                bitmap = font32up;
        } else {
                i -= 50;
                bitmap = font82up;
        }
        bitmap = &bitmap[(i << 2) + i];

        sysArgs[0] = 0x3f; // XXX color
        *(int*)(sysArgs+4) = _ScreenPos;
        sysFn = SYS_VDrawBits_134;

        for (i = 5; i > 0; --i, bitmap++) {
                sysArgs[2] = __lookup(0, bitmap) ^ 0xff;
                __syscall(203);
                sysArgs[4]++;
        }
        sysArgs[2] = 0;
        __syscall(203);

        _ScreenPos += 6;

        return 0;
}

