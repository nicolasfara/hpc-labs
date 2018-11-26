/* */
/****************************************************************************
 *
 * cuda-odd-even.cu - Odd-even sort with CUDA
 *
 * Written in 2017 by Moreno Marzolla <moreno.marzolla(at)unibo.it>
 * Last modified in 2018 by Moreno Marzolla
 *
 * To the extent possible under law, the author(s) have dedicated all 
 * copyright and related and neighboring rights to this software to the 
 * public domain worldwide. This software is distributed without any warranty.
 *
 * You should have received a copy of the CC0 Public Domain Dedication
 * along with this software. If not, see 
 * <http://creativecommons.org/publicdomain/zero/1.0/>. 
 *
 * ---------------------------------------------------------------------------
 *
 * Compile with:
 * nvcc cuda-odd-even.cu -o cuda-odd-even
 *
 * Run with:
 * ./cuda-odd-even [len]
 *
 * Example:
 * ./cuda-odd-even
 *
 ****************************************************************************/
#include "hpc.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/* if *a > *b, swap them. Otherwise do nothing */
void cmp_and_swap( int* a, int* b )
{
    if ( *a > *b ) {
        int tmp = *a;
        *a = *b;
        *b = tmp;
    }
}

/* Odd-even transposition sort */
void odd_even_step( int* v, int n, int phase )
{
    if ( phase % 2 == 0 ) {
        /* (even, odd) comparisons */
        for (int i=0; i<n-1; i += 2 ) {
            cmp_and_swap( &v[i], &v[i+1] );
        }
    } else {
        /* (odd, even) comparisons */
        for (int i=1; i<n-1; i += 2 ) {
            cmp_and_swap( &v[i], &v[i+1] );
        }
    }
}

/**
 * Return a random integer in the range [a..b]
 */
int randab(int a, int b)
{
    return a + (rand() % (b-a+1));
}

/**
 * Fill vector x with a random permutation of the integers 0..n-1
 */
void fill( int *x, int n )
{
    int i, j, tmp;
    for (i=0; i<n; i++) {
        x[i] = i;
    }
    for(i=0; i<n-1; i++) {
        j = randab(i, n-1);
        tmp = x[i];
        x[i] = x[j];
        x[j] = tmp;
    }
}

/**
 * Check correctness of the result
 */
int check( int *x, int n )
{
    int i;
    for (i=0; i<n; i++) {
        if (x[i] != i) {
            fprintf(stderr, "Check FAILED: x[%d]=%d, expected %d\n", i, x[i], i);
            return 0;
        }
    }
    printf("Check OK\n");
    return 1;
}

int main( int argc, char *argv[] ) 
{
    int *x;
    int phase, n = 128*1024;
    const int max_len = 512*1024*1024;
    double tstart, elapsed;

    if ( argc > 2 ) {
        fprintf(stderr, "Usage: %s [len]\n", argv[0]);
        return EXIT_FAILURE;
    }

    if ( argc > 1 ) {
        n = atoi(argv[1]);
    }

    if ( n > max_len ) {
        fprintf(stderr, "FATAL: the maximum length is %d\n", max_len);
        return EXIT_FAILURE;
    }
    
    const size_t size = n * sizeof(*x);

    /* Allocate space for x on host */
    x = (int*)malloc(size); assert(x);
    fill(x, n);

    tstart = hpc_gettime();
    for (phase = 0; phase < n; phase++) {
        odd_even_step(x, n, phase);
    }
    elapsed = hpc_gettime() - tstart;
    printf("Sorted %d elements in %f seconds\n", n, elapsed);        

    /* Check result */
    check(x, n);

    /* Cleanup */
    free(x);

    return EXIT_SUCCESS;
}
