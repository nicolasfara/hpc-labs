/* */
/****************************************************************************
 *
 * mpi-my-bcast.c - Broadcast using point-to-point communications
 *
 * Written in 2017 by Moreno Marzolla <moreno.marzolla(at)unibo.it>
 * Last updated in 2018 by Moreno Marzolla
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
 * Compile with
 * mpicc -std=c99 -Wall -Wpedantic mpi-my-bcast.c -o mpi-my-bcast
 *
 * run with:
 * mpirun -n 4 ./mpi-my-bcast
 *
 ****************************************************************************/
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

void my_Bcast(int *v)
{
    int my_rank, comm_sz;
    
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);    
    /* [TODO] Complete this function */
}


int main( int argc, char *argv[] )
{
    int my_rank;
    int v;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    if ( 0 == my_rank ) {
        v = 999; /* only process 0 sets the value to be sent */
    } else {
        v = -1; /* all other processes set v to -1; if everything goes well, the value will be overwritten with the value received from the master */ 
    }

    my_Bcast(&v);

    if ( v == 999 ) {
        printf("OK: ");
    } else {
        printf("ERROR: ");
    }
    printf("Process %d has %d\n", my_rank, v);

    MPI_Finalize();

    return 0;
}
