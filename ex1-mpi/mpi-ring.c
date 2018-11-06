/* */
/****************************************************************************
 *
 * mpi-ring.c - Ring communication with MPI
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
 * mpicc -std=c99 -Wall -Wpedantic mpi-ring.c -o mpi-ring
 *
 * Run with:
 * mpirun -n 4 ./mpi-ring
 *
 ****************************************************************************/

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main( int argc, char *argv[] )
{
  int v, my_rank, comm_sz, K = 10;

  MPI_Init( &argc, &argv );	
  MPI_Comm_rank( MPI_COMM_WORLD, &my_rank );
  MPI_Comm_size( MPI_COMM_WORLD, &comm_sz );

  if ( argc > 1 ) {
    K = atoi(argv[1]);
  }
  /* [TODO] Rest of the code here... */

  if (my_rank == 0) {
    v = 0;

    while (K > 0) {

    }

  } else {

  }

  MPI_Finalize();		
  return 0;
}


// vim: set nofoldenable :
