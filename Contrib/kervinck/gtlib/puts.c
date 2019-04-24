
#include "stdio.h"

int puts(char* s) {
        unsigned char* ss = (unsigned char*)s;
        for (; *ss != '\0'; ss++) {
                putc(*ss);
        }
        return 0;
}

