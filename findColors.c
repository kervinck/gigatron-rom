
/*
 *  Find 9x7 arrangement of colors in smooth gradient
 *  Use all colors 1-63 from the 64 color RRGGBB palette (0 is black)
 *  Use a heuristic local search
 */

#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

const int X = 9;
const int Y = 7;

static int channelDiff(int a, int b)
{
        a &= 3;
        b &= 3;
        return (a - b) * (a - b);
}

static int colorDiff(int a, int b)
{
        return channelDiff(a, b)
             + channelDiff(a>>2, b>>2)
             + channelDiff(a>>4, b>>4);
}

double calcError(int M[X*Y])
{
        int error = 0;
        for (int i=0; i<63; i++) {
                int x = i%X, y = i/X;
                if (x > 0) error += colorDiff(M[i], M[i-1]);
                if (y > 0) error += colorDiff(M[i], M[i-X]);
                // No need to count every pair twice
        }
        return (double)error / (2.0 * (X-1) * (Y-1));
}

// Shift columns, rows, or both. Wrap around.
double tryShift(int M[X*Y], double mError)
{
        for (int col=0; col<X; col++)
        for (int row=0; row<Y; row++) {
                int N[X*Y];

                for (int i=0; i<X; i++)
                for (int j=0; j<Y; j++)
                        N[i+j*X] = M[(i+col)%X + (j+row)%Y * X];

                double nError = calcError(N);
                if (nError < mError) {
                        memcpy(M, N, sizeof(N));
                        return nError;
                }

        }
        return mError;
}

// Swap 2 places
double trySwap2(int M[X*Y], double mError)
{
        for (int i=0; i<63-1; i++)
        for (int j=i+1; j<63; j++) {
                int N[X*Y];
                memcpy(N, M, sizeof(N));
                N[i] = M[j];
                N[j] = M[i];

                double nError = calcError(N);
                if (nError < mError) {
                        memcpy(M, N, sizeof(N));
                        return nError;
                }
        }
        return mError;
}

// Swap 3 places
double trySwap3(int M[X*Y], double mError)
{
        for (int i=0; i<63; i++)
        for (int j=0; j<63; j++) if (j!=i)
        for (int k=0; k<63; k++) if (k!=i && k!=j) {
                int N[X*Y];

                memcpy(N, M, sizeof(N));
                N[i] = M[j];
                N[j] = M[k];
                N[k] = M[i];

                double nError = calcError(N);
                if (nError < mError) {
                        memcpy(M, N, sizeof(N));
                        return nError;
                }
        }
        return mError;
}

// Swap 4 places
double trySwap4(int M[X*Y], double mError)
{
        for (int i=0; i<63; i++)
        for (int j=0; j<63; j++) if (j!=i)
        for (int k=0; k<63; k++) if (k!=i && k!=j)
        for (int l=0; l<63; l++) if (l!=i && l!=j && l!=k) {
                int N[X*Y];

                memcpy(N, M, sizeof(N));
                N[i] = M[j];
                N[j] = M[k];
                N[k] = M[l];
                N[l] = M[i];
                double nError = calcError(N);
                if (nError < mError) {
                        memcpy(M, N, sizeof(N));
                        return nError;
                }
        }
        return mError;
}

// Swap 5 places
double trySwap5(int M[X*Y], double mError)
{
        for (int i=0; i<63; i++)
        for (int j=0; j<63; j++) if (j!=i)
        for (int k=0; k<63; k++) if (k!=i && k!=j)
        for (int l=0; l<63; l++) if (l!=i && l!=j && l!=k)
        for (int m=0; m<63; m++) if (m!=i && m!=j && m!=k && m!=l) {
                int N[X*Y];

                memcpy(N, M, sizeof(N));
                N[i] = M[j];
                N[j] = M[k];
                N[k] = M[l];
                N[l] = M[m];
                N[m] = M[i];
                double nError = calcError(N);
                if (nError < mError) {
                        memcpy(M, N, sizeof(N));
                        return nError;
                }
        }
        return mError;
}

// Swap 6 places (this is really slow)
double trySwap6(int M[X*Y], double mError)
{
        for (int i=0; i<63; i++)
        for (int j=0; j<63; j++) if (j!=i)
        for (int k=0; k<63; k++) if (k!=i && k!=j)
        for (int l=0; l<63; l++) if (l!=i && l!=j && l!=k)
        for (int m=0; m<63; m++) if (m!=i && m!=j && m!=k && m!=l)
        for (int n=0; n<63; n++) if (n!=i && n!=j && n!=k && n!=l && n!=m) {
                int N[X*Y];

                memcpy(N, M, sizeof(N));
                N[i] = M[j];
                N[j] = M[k];
                N[k] = M[l];
                N[l] = M[m];
                N[m] = M[n];
                N[n] = M[i];
                double nError = calcError(N);
                if (nError < mError) {
                        memcpy(M, N, sizeof(N));
                        return nError;
                }
        }
        return mError;
}

// Print in Python format
void print(int M[X*Y], double mError)
{
        printf("colors = [\n");
        for (int row=0; row<Y; row++) {
                for (int col=0; col<X; col++)
                        printf(" %2d,", M[row*X + col]);
                printf(" #");
                for (int col=0; col<X; col++) {
                        int i = row*X + col;
                        static char art[4] = ".-+#";
                        putchar(' ');
                        putchar(art[ M[i]    &3]);
                        putchar(art[(M[i]>>2)&3]);
                        putchar(art[(M[i]>>4)&3]);
                }
                putchar('\n');
        }
        printf("] # error %lf\n", sqrt(mError));
}

double shuffle(int M[X*Y])
{
        for (int i=0; i<63; i++) {
                int j = i + (rand() % (63-i));
                int t = M[i];
                M[i] = M[j];
                M[j] = t;
        }
        return calcError(M);
}

int main(void)
{
        int M[X*Y];
        double mError;

        srand(time(NULL));

        for (int i=0; i<63; i++)
                M[i] = 1+i;

        /*
         *  First find a reasonable starting point
         *  Start randomly, try simple optimizations until a local minimum
         *  Throw away the first 1000 results
         *  After that, continue with the first that is better than all others
         */
        int count = 0;
        double bestError = DBL_MAX;
        double currentError;
        for (;;) {
                // Randomize
                double currentError = shuffle(M);
                // Find local optimimum with just shifts and swaps
                for (;;) {
                        mError = currentError;
                        currentError = tryShift(M, mError);
                        if (currentError < mError)
                                continue;
                        currentError = trySwap2(M, mError);
                        if (currentError < mError)
                                continue;
                        break;
                }
                count++;
                // Simple local optimum
                if (mError < bestError) {
                        bestError = mError;
                        printf("best %lf count %d\n", sqrt(bestError), count);
                        if (count >= 1000)
                                break;
                }
        }

        /*
         *  Now for real
         */
        currentError = mError;
        for (;;) {
                mError = currentError;

                print(M, mError);

                puts("tryShift");
                currentError = tryShift(M, mError);
                if (currentError < mError)
                        continue;

                puts("trySwap2");
                currentError = trySwap2(M, mError);
                if (currentError < mError)
                        continue;

                puts("trySwap3");
                currentError = trySwap3(M, mError);
                if (currentError < mError)
                        continue;

                puts("trySwap4");
                currentError = trySwap4(M, mError);
                if (currentError < mError)
                        continue;

                puts("trySwap5");
                currentError = trySwap5(M, mError);
                if (currentError < mError)
                        continue;

                puts("trySwap6");
                currentError = trySwap6(M, mError);
                if (currentError < mError)
                        continue;

                break; // Stop when nothing works
        }

        return 0;
}

