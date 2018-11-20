/* */
/****************************************************************************
 *
 * mpi-send-col.c - Exchange columns using MPI datatype
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
 * mpicc -std=c99 -Wall -Wpedantic mpi-send-col.c -o mpi-send-col.c
 *
 * Run with:
 * mpirun -n 2 ./mpi-send-col.c
 *
 ****************************************************************************/
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

/* Matrix size */
#define SIZE 4

/* Initialize matrix m with the values k, k+1, k+2, ..., from left to
   right, top to bottom. m must point to an already allocated block of
   (size+2)*size integers. The first and last column of m is the halo,
   which is set to -1. */
void init_matrix( int *m, int size, int k )
{
  int i, j;
  for (i=0; i<size; i++) {
    for (j=0; j<size+2; j++) {
      if ( j==0 || j==size+1) {
        m[i*(size+2)+j] = -1;
      } else {
        m[i*(size+2)+j] = k;
        k++;
      }
    }
  }
}

void print_matrix( int *m, int size )
{
  int i, j;
  for (i=0; i<size; i++) {
    for (j=0; j<size+2; j++) {
      printf("%3d ", m[i*(size+2)+j]);
    }
    printf("\n");
  }
}

int main( int argc, char *argv[] )
{
  int my_rank, comm_sz;
  int my_mat[SIZE][SIZE+2];

  MPI_Init( &argc, &argv );
  MPI_Comm_rank( MPI_COMM_WORLD, &my_rank );
  MPI_Comm_size( MPI_COMM_WORLD, &comm_sz );

  if ( 0 == my_rank && 2 != comm_sz ) {
    fprintf(stderr, "You must execute exactly 2 processes\n");
    MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
  }
  if ( 0 == my_rank ) {
    init_matrix(&my_mat[0][0], SIZE, 0);
  } else if (1 == my_rank) {
    init_matrix(&my_mat[0][0], SIZE, SIZE*SIZE);
  }

  /* [TODO] Exchange borders here */

  /* Print the matrices after the exchange; to do so without
     interference we must use this funny strategy: process 0 prints,
     then the processes synchronize, then process 1 prints. */
  if ( 0 == my_rank ) {
    printf("\n\nProcess 0:\n");
    print_matrix(&my_mat[0][0], SIZE);
  }
  MPI_Barrier(MPI_COMM_WORLD);
  if ( 1 == my_rank ) {
    printf("\n\nProcess 1:\n");
    print_matrix(&my_mat[0][0], SIZE);
  }

  MPI_Finalize();
  return EXIT_SUCCESS;
}

// vim: set nofoldenable :
