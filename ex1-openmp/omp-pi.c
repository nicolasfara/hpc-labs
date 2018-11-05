/* */
/****************************************************************************
 *
 * omp-pi.c - Monte Carlo approximation of PI
 *
 * Written in 2017 by Moreno Marzolla <moreno.marzolla(at)unibo.it>
 * Modified in 2018 by Moreno Marzolla
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
 * This program implements a serial algorithm for computing the
 * approximate value of PI using a Monte Carlo method.
 *
 * Compile with:
 *
 * gcc -std=c99 -fopenmp -Wall -Wpedantic omp-pi.c -o omp-pi -lm
 *
 * Run with:
 *
 * OMP_NUM_THREADS=4 ./omp-pi 20000
 *
 ****************************************************************************/
/* The rand_r() function is available only if _XOPEN_SOURCE is defined */
#define _XOPEN_SOURCE
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h> /* for fabs */

/* Generate |n| random points within the square with corners (-1, -1),
   (1, 1); return the number of points that fall inside the circle
   centered ad the origin with radius 1. */
unsigned int generate_points( unsigned int n )
{
    unsigned int i, ninside = 0;
    /* The C function rand() is not thread-safe, and therefore can not
       be used with OpenMP. We use rand_r() with an explicit per-thread seed. */
    unsigned int my_seed = 17 + 19 * omp_get_thread_num();

#pragma omp parallel for reduction(+:ninside) default(none) private(my_seed) shared(n)
    for (i = 0; i < n; i++) {
        /* Generate two random values in the range [-1, 1] */
        const double x = (2.0 * rand_r(&my_seed)/(double)RAND_MAX) - 1.0;
        const double y = (2.0 * rand_r(&my_seed)/(double)RAND_MAX) - 1.0;
        if ( x*x + y*y <= 1.0 ) {
            ninside++;
        }
    }
    return ninside;
}

int main( int argc, char *argv[] )
{
    unsigned int npoints = 10000;
    int ninside;
    const double PI_EXACT = 3.14159265358979323846;

    if ( argc > 2 ) {
        fprintf(stderr, "Usage: %s [npoints]\n", argv[0]);
        return EXIT_FAILURE;
    }

    if ( argc > 1 ) {
        npoints = atol(argv[1]);
    }

    printf("Generating %u points...\n", npoints);
    const double tstart = omp_get_wtime();
    ninside = generate_points(npoints);
    const double elapsed = omp_get_wtime() - tstart;
    double pi_approx = 4.0 * ninside / (double)npoints;
    printf("PI approximation %f, exact %f, error %f%%\n", pi_approx, PI_EXACT, 100.0*fabs(pi_approx - PI_EXACT)/PI_EXACT);
    printf("Elapsed time: %f\n", elapsed);

    return EXIT_SUCCESS;
}

// vim: set nofoldenable ts=4 sw=4 :
