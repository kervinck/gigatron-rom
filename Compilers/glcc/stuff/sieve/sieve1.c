#include <stdio.h>
#include <gigatron/sys.h>
#define TIMER 1


/** This is a minor modification of the genuine C program of the sieve benchmark.
    Loop conditions and certain expressions have been made optimizer friendly. **/

#define true 1
#define false 0
#define size 8190
#define sizepl 8191
char flags[sizepl];

main() {
    int i, prime, k, count, iter;
#if TIMER
    unsigned int ticks = 0;
    frameCount = 0;
#endif
    printf("10 iterations\n");
#ifdef MODE
# if MODE == 4
    videoTop_v5 = 238;
# else
    SYS_SetMode(MODE);
# endif
#endif
    for (iter = 1; iter <= 10; iter ++) {
        count=0 ; 
	for (i = 0; i != sizepl; i++)
	    flags[i] = true; 
        for (i = 0; i != sizepl; i++) { 
	    if (flags[i]) {
                prime = i + i + 3; 
                k = prime + i; 
                while (size - k >= 0) { 
                    flags[k] = false; 
                    k += prime; 
                }
                count = count + 1;
            }
        }
#if TIMER
	ticks += frameCount;
	frameCount = 0;
#endif
    }
#ifdef MODE
# if MODE == 4
    videoTop_v5 = 0;
# else
    SYS_SetMode(-1);
# endif
#endif
    printf("\n%d primes", count);
#if TIMER
    printf("\n%d %d/60 seconds", ticks/60, ticks % 60);
#endif
}

/* Local Variables: */
/* mode: c */
/* c-basic-offset: 4 */
/* indent-tabs-mode: () */
/* End: */
