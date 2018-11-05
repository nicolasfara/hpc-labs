/* */
/****************************************************************************
 *
 * omp-dot.c - Dot product
 *
 * Written in 2018 by Moreno Marzolla <moreno.marzolla(at)unibo.it>
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
 * Parallel computation of the dot product of two arrays of length
 * n. Compile with:
 *
 * gcc -fopenmp -std=c99 -Wall -Wpedantic omp-dot.c -o omp-dot
 *
 * Run with:
 *
 * ./omp-dot [n]
 *
 * Example:
 *
 * ./omp-dot 1000000
 *
 ****************************************************************************/
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

void fill( int *v1, int *v2, size_t n )
{
    const int seq1[3] = { 3, 7, 18};
    const int seq2[3] = {12, 0, -2};
    size_t i;
    for (i=0; i<n; i++) {
        v1[i] = seq1[i%3];
        v2[i] = seq2[i%3];
    }
}

int main( int argc, char *argv[] )
{
    size_t n = 10*1024*1024; /* array length */
    const size_t n_max = 512*1024*1024; /* max length */
    int dotprod, expect;
    int *v1, *v2;
    size_t i;
    char *end = NULL;

    if ( argc > 2 ) {
        fprintf(stderr, "Usage: %s [n]\n", argv[0]);
        return EXIT_FAILURE;
    }

    if ( argc > 1 ) {
        n = strtol(argv[1], &end, 10);
    }

    if ( n > n_max ) {
        fprintf(stderr, "The array length must be lower than %lu\n", (unsigned long)n_max);
        return EXIT_FAILURE;
    }

    printf("Initializing array of length %lu\n", (unsigned long)n);
    v1 = (int*)malloc( n*sizeof(v1[0]));
    v2 = (int*)malloc( n*sizeof(v2[0]));
    fill(v1, v2, n);

    expect = (n % 3 == 0 ? 0 : 36);

    const double tstart = omp_get_wtime();

    dotprod = 0;
#pragma omp parallel for reduction(+:dotprod) default(none) shared(v1, v2, n)
    for (i=0; i<n; i++) {
        dotprod += v1[i] * v2[i];
    }
    const double elapsed = omp_get_wtime() - tstart;

    if ( dotprod == expect ) {
        printf("Test OK\n");
    } else {
        printf("Test FAILED: expected %d, got %d\n", expect, dotprod);
    }
    printf("Elapsed time: %f\n", elapsed);
    free(v1);
    free(v2);

    return EXIT_SUCCESS;
}

// vim: set nofoldenable :
