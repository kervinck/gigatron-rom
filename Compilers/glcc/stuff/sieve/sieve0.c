#include <stdio.h>
#include <gigatron/sys.h>
#include <gigatron/libc.h>
#include <time.h>

#ifndef TIMER
# define TIMER 1
#endif
#ifndef PRINT
# define PRINT 1
#endif

/** This is the pristine C program of the sieve benchmark.
    The only additions are the #include directives above
    and the timing code enabled by the preprocessor symbol TIMER. **/

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
#if PRINT
    printf("10 iterations\n");
#endif
    for (iter = 1; iter <= 10; iter ++) {
        count=0 ; 
	for (i = 0; i <= size; i++)
	    flags[i] = true; 
        for (i = 0; i <= size; i++) { 
	    if (flags[i]) {
                prime = i + i + 3; 
                k = i + prime; 
                while (k <= size) {
                    flags[k] = false; 
                    k += prime; 
                }
                count = count + 1;
            }
        }
    }
#if PRINT
    printf("\n%d primes", count);
#endif
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
