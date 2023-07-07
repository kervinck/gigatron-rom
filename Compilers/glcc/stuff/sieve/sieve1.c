#include <stdio.h>
#include <string.h>
#include <gigatron/sys.h>
#include <gigatron/libc.h>
#include <time.h>

#ifndef TIMER
# define TIMER 1
#endif
#ifndef MEMSET
# define MEMSET 1
#endif

/** This is a minor modification of the pristine C program of the
    sieve benchmark. Loop conditions and certain expressions have
    been made Gigatron friendly. Table clearing is conditionally done
    with memset (which is proper ANSI C 1989). */

#define true 1
#define false 0
#define size 8190
#define sizepl 8191

char flags[sizepl];

main() {
    int i, prime, k, count, iter;
#if TIMER
    unsigned int ticks = _clock();
#endif
    printf("10 iterations\n");
    for (iter = 1; iter <= 10; iter ++) {
        count = 0;
#if MEMSET
        /* memset is a legit ANSI C 1989 function
           and clears up to 32 bytes per scanline */
        memset(flags, true, sizepl);
#else
        /* simple pointer loop */
        { char *p = flags; while(p != flags+sizepl) *p++ = true; }
#endif
        for (i = 0; i != sizepl; i++) { 
	    if (flags[i]) {
                prime = i + i + 3; 
                k = prime + i; 
                while (size >= k) { 
                    flags[k] = false; 
                    k += prime; 
                }
                count = count + 1;
            }
        }
    }
    printf("\n%d primes", count);
#if TIMER
    ticks = _clock() - ticks;
    printf("\n%d %d/60 seconds", ticks/60, ticks % 60);
#endif
}

/* Local Variables: */
/* mode: c */
/* c-basic-offset: 4 */
/* indent-tabs-mode: () */
/* End: */
