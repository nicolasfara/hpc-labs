/* */
/****************************************************************************
 *
 * cat-map-rectime.c - Compute the minimum recurrence time of Arnold's
 * cat map for a given image size
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
 * This program computes the recurrence time of Arnold's cat map for 
 * an image of given size (n, n).
 *
 * Compile with:
 * gcc -std=c99 -Wall -Wpedantic -fopenmp omp-cat-map-rectime.c -o omp-cat-map-rectime
 *
 * Run with:
 * ./omp-cat-map-rectime [n]
 *
 * Example:
 * ./omp-cat-map-rectime 1024 
 *
 ****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <omp.h>

/* Compute the Greatest Common Divisor of integers a>0 and b>0 */
int gcd(int a, int b)
{
    assert(a>0);
    assert(b>0);

    while ( b != a ) {
        if (a>b) {
            a = a-b;
        } else {
            b = b-a;
        }
    }
    return a;
}

/* compute the Least Common Multiple of integers a>0 and b>0 */
int lcm(int a, int b)
{
    assert(a>0);
    assert(b>0);
    return (a / gcd(a, b))*b;
}

/**
 * Compute the recurrence time of Arnold's cat map applied to an image
 * of size (n*n). The idea is the following. Each point (x,y) requires
 * k(x,y) iterations to return to its starting position. Therefore,
 * the minimum recurrence time for the whole image is the Least Common
 * Multiple of all recurrence times k(x,y), for each pixel 0 <= x < n,
 * 0 <= y < n.
 */
int cat_map_rectime( int n )
{
    /* [TODO] Implement this function; start with a working serial
       version, then parallelize. */
    return 0;
}

int main( int argc, char* argv[] )
{
    int n, k;
    if ( argc != 2 ) {
        fprintf(stderr, "Usage: %s image_size\n", argv[0]);
        return EXIT_FAILURE;
    }
    n = atoi(argv[1]);
    const double tstart = omp_get_wtime();
    k = cat_map_rectime(n);
    const double elapsed = omp_get_wtime() - tstart;
    printf("Recurrence time for an image of size %d x %d = %d\n", n, n, k);
    printf("Elapsed time: %f\n", elapsed);
    return EXIT_SUCCESS; 
}
