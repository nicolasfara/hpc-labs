/* */
/****************************************************************************
 *
 * cuda-matsum.cu - Dense matrix-matrix addition with CUDA
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
 * Matrix sum with CUDA.
 *
 * Compile with:
 * nvcc cuda-matsum.cu -o cuda-matsum -lm
 *
 * Run with:
 * ./cuda-matsum
 *
 ****************************************************************************/
#include "hpc.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>


void matsum( float *p, float *q, float *r, int n )
{
    /* [TODO] Modify the body of this function to
       - allocate memory on the device
       - copy p and q to the device
       - call an appropriate kernel
       - copy the result back from the device to the host
       - free memory on the device
    */
    int i, j;
    for (i=0; i<n; i++) {
        for (j=0; j<n; j++) {
            r[i*n + j] = p[i*n + j] + q[i*n + j];
        }
    }
}

/* Initialize square matrix p of size nxn */
void fill( float *p, int n )
{
    int i, j, k=0;
    for (i=0; i<n; i++) {
        for (j=0; j<n; j++) {
            p[i*n+j] = k;
            k = (k+1) % 1000;
        }
    }
}

/* Check result */
int check( float *r, int n )
{
    int i, j, k = 0;
    for (i=0; i<n; i++) {
        for (j=0; j<n; j++) {
            if (fabsf(r[i*n+j] - 2.0*k) > 1e-5) {
                fprintf(stderr, "Check FAILED: r[%d][%d] = %f, expeted %f\n", i, j, r[i*n+j], 2.0*k);
                return 0;
            }
            k = (k+1) % 1000;
        }
    }
    printf("Check OK\n");
    return 1;
}

int main( int argc, char *argv[] ) 
{
    float *p, *q, *r;
    int n = 1024;
    const int max_n = 5000;
    
    if ( argc > 2 ) {
        fprintf(stderr, "Usage: %s [n]\n", argv[0]);
        return EXIT_FAILURE;
    }

    if ( argc > 1 ) {
        n = atoi(argv[1]);
    }

    if ( n > max_n ) {
        fprintf(stderr, "FATAL: the maximum allowed matrix size is %d\n", max_n);
        return EXIT_FAILURE;
    }
    
    const size_t size = n*n*sizeof(*p);

    /* Allocate space for p, q, r */
    p = (float*)malloc(size); assert(p);
    fill(p, n);
    q = (float*)malloc(size); assert(q);
    fill(q, n);
    r = (float*)malloc(size); assert(r);

    const double tstart = hpc_gettime();
    matsum(p, q, r, n);
    const double elapsed = hpc_gettime() - tstart;

    printf("Elapsed time (including data movement): %f\n", elapsed);
    
    /* Check result */
    check(r, n);

    /* Cleanup */
    free(p); free(q); free(r);
    
    return EXIT_SUCCESS;
}
