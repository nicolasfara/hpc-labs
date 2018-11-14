/* */
/****************************************************************************
 *
 * bbox-gen.c - Generate an input file for the mpi-bbox.c program
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
 * gcc -ansi -Wall -Wpedantic bbox-gen.c -o bbox-gen
 *
 * To generate 1000 random rectangles, run:
 * ./bbox-gen 1000 > bbox-1000.in
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
        printf("Usage: %s n\n", argv[0]);
        return -1;
    }
    n = atoi( argv[1] );
    printf("%d\n", n);
    for (i=0; i<n; i++) {
        float x1 = randab(0, 1000);
        float y1 = randab(0, 1000);
        float x2 = x1 + randab(10, 100);
        float y2 = y1 + randab(10, 100);
        printf("%f %f %f %f\n", x1, y1, x2, y2);
    }
    return 0;
}


// vim: set nofoldenable :
