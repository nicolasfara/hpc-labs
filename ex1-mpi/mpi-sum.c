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

  /* Comunicazione porzioni array */
  if (0 == my_rank) {
    for (int p = 1; p < comm_sz; p++) {
      const int start = p * n / comm_sz;
      const int end = (p + 1) * n / comm_sz;
      const int size = end - start;

      printf("Proc 0 send [%d, %d]\n", start, end);

      MPI_Send(master_array + start, size, MPI_FLOAT, p, 0, MPI_COMM_WORLD);
    }

    s = sum(master_array, n / comm_sz);

    for (int p = 1; p < comm_sz; p++) {
      float remote_s;
      MPI_Recv(&remote_s, 1, MPI_FLOAT, p, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      s += remote_s;
    }
  } else {
      const int start = my_rank * n / comm_sz;
      const int end = (my_rank + 1) * n / comm_sz;
      const int size = end - start;
      float *local_array = (float *)malloc(sizeof(float) * size);

      printf("Proc %d receive [%d, %d]\n", my_rank, start, end);
      MPI_Recv(local_array, size, MPI_FLOAT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      float local_s = sum(local_array, size);

      MPI_Send(&local_s, 1, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
      free(local_array);
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

  free(master_array);

  MPI_Finalize();
  return 0;
}

// vim: set nofoldenable :
