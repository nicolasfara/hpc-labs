/* */
/****************************************************************************
 *
 * cuda-dot.cu - Dot product with CUDA
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
 * nvcc cuda-dot.cu -o cuda-dot -lm
 *
 * Run with:
 * ./cuda-dot [len]
 *
 * Example:
 * ./cuda-dot
 *
 ****************************************************************************/
#include "hpc.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>


double dot( double *x, double *y, int n )
{
    /* [TODO] modify this function so that (part of) the dot product
       computation is executed on the GPU. */
    double result = 0.0;
    for (int i = 0; i < n; i++) {
        result += x[i] * y[i];
    }
    return result;
}

void vec_init( double *x, double *y, int n )
{
    int i;
    const double tx[] = {1.0/64.0, 1.0/128.0, 1.0/256.0};
    const double ty[] = {1.0, 2.0, 4.0};
    const size_t arrlen = sizeof(tx)/sizeof(tx[0]);

    for (i=0; i<n; i++) {
        x[i] = tx[i % arrlen];
        y[i] = ty[i % arrlen];
    }
}

int main( int argc, char* argv[] ) 
{
    double *x, *y, result;
    int n = 1024*1024;
    const int max_len = 128 * n;

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

    const size_t size = n*sizeof(*x);

    /* Allocate space for host copies of x, y */
    x = (double*)malloc(size); assert(x);
    y = (double*)malloc(size); assert(y);
    vec_init(x, y, n);

    printf("Computing the dot product of %d elements... ", n);
    result = dot(x, y, n);
    printf("result=%f\n", result);

    const double expected = ((double)n)/64;

    /* Check result */
    if ( fabs(result - expected) < 1e-5 ) {
        printf("Check OK\n");
    } else {
        printf("Check FAILED: got %f, expected %f\n", result, expected);
    }
    
    /* Cleanup */
    free(x); free(y);
    
    return EXIT_SUCCESS;
}
