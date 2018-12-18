/* */
/****************************************************************************
 *
 * simd-matmul.c - Dense matrix-matrix multiply using vector datatypes
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
 * --------------------------------------------------------------------------
 *
 * Compile with:
 * gcc -march=native -std=c99 -Wall -Wpedantic -D_XOPEN_SOURCE=600 simd-matmul.c -o simd-matmul
 *
 * Run with:
 * ./simd-matmul
 *
 ****************************************************************************/

/* The following #define is required by posix_memalign() */
#define _XOPEN_SOURCE 600

#include "hpc.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>  /* for assert() */
#include <strings.h> /* for bzero() */

typedef double v2d __attribute__((vector_size(16)));
#define VLEN (sizeof(v2d)/sizeof(double))

/* Fills n x n square matrix m */
void fill( double* m, int n )
{
    int i, j;
    for (i=0; i<n; i++) {
        for (j=0; j<n; j++) {
            m[i*n + j] = (i%10+j) / 10.0;
        }
    }
}

/* compute r = p * q, where p, q, r are n x n matrices. */
void scalar_matmul( const double *p, const double* q, double *r, int n)
{
    int i, j, k;

    for (i=0; i<n; i++) {
        for (j=0; j<n; j++) {
            double s = 0.0;
            for (k=0; k<n; k++) {
                s += p[i*n + k] * q[k*n + j];
            }
            r[i*n + j] = s;
        }
    }
}

/* Cache-efficient computation of r = p * q, where p. q, r are n x n
   matrices. This function allocates (and then releases) an additional n x n
   temporary matrix. */
void scalar_matmul_tr( const double *p, const double* q, double *r, int n)
{
    int i, j, k;
    double *qT = (double*)malloc( n * n * sizeof(*qT) );

    /* transpose q, storing the result in qT */
    for (i=0; i<n; i++) {
        for (j=0; j<n; j++) {
            qT[j*n + i] = q[i*n + j];
        }
    }    

    /* multiply p and qT row-wise */
    for (i=0; i<n; i++) {
        for (j=0; j<n; j++) {
            double s = 0.0;
            for (k=0; k<n; k++) {
                s += p[i*n + k] * qT[j*n + k];
            }
            r[i*n + j] = s;
        }
    }

    free(qT);
}

/* SIMD version of the cache-efficient matrix-matrix multiply above.
   This function requires that n is a multiple of the SIMD vector
   length VLEN */
void simd_matmul_tr( const double *p, const double* q, double *r, int n)
{
    /* [TODO] Implement this function */
}

int main( int argc, char* argv[] )
{
    int n = 512;
    double *p, *q, *r;
    double tstart, tstop;
    int ret;

    if ( argc > 2 ) {
        fprintf(stderr, "Usage: %s [n]\n", argv[0]);
        return EXIT_FAILURE;
    }

    if ( argc > 1 ) {
        n = atoi(argv[1]);
    }

    if ( 0 != n % VLEN ) {
        fprintf(stderr, "ERROR: the matrix size must be a multiple of %d\n", (int)VLEN);
        return EXIT_FAILURE;
    }

    const size_t size = n*n*sizeof(*p);

    ret = posix_memalign((void**)&p, __BIGGEST_ALIGNMENT__,  size); 
    assert( 0 == ret );
    ret = posix_memalign((void**)&q, __BIGGEST_ALIGNMENT__,  size); 
    assert( 0 == ret );
    ret = posix_memalign((void**)&r, __BIGGEST_ALIGNMENT__,  size); 
    assert( 0 == ret );

    fill(p, n);
    fill(q, n);
    printf("\nMatrix size: %d x %d\n\n", n, n);

    tstart = hpc_gettime();
    scalar_matmul(p, q, r, n);
    tstop = hpc_gettime();
    printf("Scalar\t\tr[0][0] = %f, Execution time = %f\n", r[0], tstop - tstart);

    bzero(r, size);

    tstart = hpc_gettime();
    scalar_matmul_tr(p, q, r, n);
    tstop = hpc_gettime();
    printf("Transposed\tr[0][0] = %f, Execution time = %f\n", r[0], tstop - tstart);

    bzero(r, size);

    tstart = hpc_gettime();
    simd_matmul_tr(p, q, r, n);
    tstop = hpc_gettime();
    printf("SIMD transposed\tr[0][0] = %f, Execution time = %f\n", r[0], tstop - tstart);

    free(p);
    free(q);
    free(r);
    return EXIT_SUCCESS;
}
