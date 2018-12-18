/* */
/****************************************************************************
 *
 * knapsack-gen.c - generate instances for the knapsack solver
 *
 * Written in 2016, 2017 by Moreno Marzolla <moreno.marzolla(at)unibo.it>
 *
 * To the extent possible under law, the author(s) have dedicated all 
 * copyright and related and neighboring rights to this software to the 
 * public domain worldwide. This software is distributed without any warranty.
 *
 * You should have received a copy of the CC0 Public Domain Dedication
 * along with this software. If not, see 
 * <http://creativecommons.org/publicdomain/zero/1.0/>. 
 *
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h> /* for time() */

int main( int argc, char* argv[] )
{
    int i, C, n;
    if ( argc != 3 ) {
        fprintf(stderr, "Usage: %s knapsack_capacity num_items\n", argv[0]);
        return EXIT_FAILURE;
    }
    srand(time(NULL));
    C = atoi(argv[1]);
    n = atoi(argv[2]);
    printf("%d\n%d\n", C, n);
    for ( i=0; i<n; i++ ) {
        printf("%d %f\n", 1 + rand() % (C/2), ((float)rand())/RAND_MAX * 10.0);
    }
    return EXIT_SUCCESS;
}
