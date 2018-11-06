/* */
/****************************************************************************
 *
 * mpi-sum.c - Sum the content of an array using send/receive
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
 * Compile with:
 * mpicc -std=c99 -Wall -Wpedantic mpi-sum.c -o mpi-sum
 *
 * Run with:
 * mpirun -n 4 ./mpi-sum
 *
 ****************************************************************************/
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

/* Compute the sum of all elements of array |v| of length |n| */
float sum(float *v, int n)
{
    float sum = 0;
    int i;
    
    for (i=0; i<n; i++) {
        sum += v[i];
    }
    return sum;
}

/* Fill array v of length n; store into *expected_sum the sum of the
   content of v */
void fill(float *v, int n, float *expected_sum)
{
    const float vals[] = {1, -1, 2, -2, 0};
    const int NVALS = sizeof(vals)/sizeof(vals[0]);
    int i;
    
    for (i=0; i<n; i++) {
        v[i] = vals[i % NVALS];
    }
    switch(i % NVALS) {
    case 1: *expected_sum = 1; break;
    case 3: *expected_sum = 2; break;
    default: *expected_sum = 0;
    }
}

int main( int argc, char *argv[] )
{
    int my_rank, comm_sz;
    float *master_array = NULL, s, expected;
    int n = 1024*1024;
    
    MPI_Init( &argc, &argv );	
    MPI_Comm_rank( MPI_COMM_WORLD, &my_rank );
    MPI_Comm_size( MPI_COMM_WORLD, &comm_sz );

    if ( argc > 1 ) {
        n = atoi(argv[1]);
    }

    /* The master initializes the array */
    if ( 0 == my_rank ) {
        master_array = (float*)malloc( n * sizeof(float) );
        fill(master_array, n, &expected);
    }

    if ( 0 == my_rank ) {
        /* [TODO] This is not a true parallel version; the master does
           everything */
        s = sum(master_array, n);
        printf("Sum=%f, expected=%g\n", s, expected);
        if (s == expected) {
            printf("Test OK\n");
        } else {
            printf("Test FAILED\n");
        }
    }
    
    MPI_Finalize();		
    return 0;
}
