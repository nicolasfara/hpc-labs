/* */
/****************************************************************************
 *
 * circles-gen.c - Generate an input file for the mpi-circles.c program
 *
 * Written in 2017 by Moreno Marzolla <moreno.marzolla(at)unibo.it>
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
 * gcc -ansi -Wall -Wpedantic circles-gen.c -o circles-gen
 *
 * To generate 1000 random rectangles, run:
 * ./circles-gen 1000 > circles-1000.in
 *
 ****************************************************************************/
#include <stdio.h>
#include <stdlib.h>

float randab(float a, float b)
{
    return a + (rand() / (float)RAND_MAX)*(b-a);
}

int main( int argc, char* argv[] )
{
    int i, n;
    if ( argc != 2 ) {
        fprintf(stderr, "Usage: %s n\n", argv[0]);
        return EXIT_FAILURE;
    }
    n = atoi( argv[1] );
    printf("%d\n", n);
    for (i=0; i<n; i++) {
        float x = randab(10, 990);
        float y = randab(10, 990);
        float r = randab(1, 10);
        printf("%f %f %f\n", x, y, r);
    }
    return EXIT_SUCCESS;
}
