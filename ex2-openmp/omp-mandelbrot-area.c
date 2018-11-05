/* */
/******************************************************************************
 *
 *  PROGRAM: Mandelbrot area
 *
 *  PURPOSE: Program to compute the area of a  Mandelbrot set.
 *           Correct answer should be around 1.510659.
 *           WARNING: this program may contain errors
 *
 *  USAGE:   Program runs without input ... just run the executable
 *            
 *  HISTORY: Written:  (Mark Bull, August 2011).
 *           Changed "complex" to "d_complex" to avoid collsion with 
 *           math.h complex type (Tim Mattson, September 2011)
 *           Code cleanup (Moreno Marzolla, Feb 2017, Oct 2018)
 *
 * --------------------------------------------------------------------------
 *
 * Compile with:
 * gcc -fopenmp -std=c99 -Wall -Wpedantic omp-mandelbrot-area.c -o omp-mandelbrot-area
 * 
 * Run with:
 * ./omp-mandelbrot-area [npoints]
 *
 ******************************************************************************/
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

const int MAXIT = 10000; /* Higher value = slower to detect points that belong to the Mandelbrot set */

struct d_complex {
    double re;
    double im;
};

/**
 * Performs the iteration z = z*z+c, until ||z|| > 2 when point is
 * known to be outside the Mandelbrot set. If loop count reaches
 * MAXIT, point is considered to be inside the set. Returns 1 iff
 * inside the set.
 */
int inside(struct d_complex c)
{
    struct d_complex z = {0.0, 0.0}, znew;
    int it;

    for ( it = 0; (it < MAXIT) && (z.re*z.re + z.im*z.im <= 4.0); it++ ) {
        znew.re = z.re*z.re - z.im*z.im + c.re;
        znew.im = 2.0*z.re*z.im + c.im;
        z = znew;
    }
    return (it >= MAXIT);
}

int main( int argc, char *argv[] )
{
    int i, j, ninside = 0, npoints = 1000;
    double area, error;
    const double eps = 1.0e-5;

    if (argc > 2) {
        fprintf(stderr, "Usage: %s [npoints]\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (argc > 1) {
        npoints = atoi(argv[1]);
    }

    printf("Using a %d x %d grid\n", npoints, npoints);

    /* Loop over grid of points in the complex plane which contains
       the Mandelbrot set, testing each point to see whether it is
       inside or outside the set. */
    const double tstart = omp_get_wtime();

    /* [TODO] Parallelize the following loop(s) */
#pragma omp parallel for schedule(runtime) reduction(+:ninside) default(none) private(j) shared(npoints)
    for (i=0; i<npoints; i++) {
        for (j=0; j<npoints; j++) {
            struct d_complex c;
            c.re = -2.0 + 2.5*i/(double)(npoints) + eps;
            c.im = 1.125*j/(double)(npoints) + eps;
            ninside += inside(c);
        }
    }

    const double elapsed = omp_get_wtime() - tstart;

    /* Compute area and error estimate and output the results */  
    area = 2.0*2.5*1.125*ninside/(((double)npoints)*npoints);
    error = area/npoints;

    printf("Area of Mandlebrot set = %12.8f +/- %12.8f\n", area, error);
    printf("Correct answer should be around 1.50659\n");
    printf("Elapsed time: %f\n", elapsed);
    return 0;
}

// vim: set nofoldenable :
