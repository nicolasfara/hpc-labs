/* */
/****************************************************************************
 *
 * mpi-pi.c - Compute a Monte Carlo approximation of PI using MPI
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
 * mpicc -std=c99 -Wall -Wpedantic mpi-pi.c -lm -o mpi-pi
 *
 * Run with:
 * mpirun -n 4 ./mpi-pi
 *
 ****************************************************************************/
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h> /* for rand() */
#include <time.h>   /* for time() */
#include <math.h>   /* for fabs() */

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* Generate |n| random points within the square with corners (-1, -1),
   (1, 1); return the number of points that fall inside the circle
   centered ad the origin with radius 1 */
int generate_points( int n ) 
{
  int i, n_inside = 0;
  for (i=0; i<n; i++) {
    const double x = (rand()/(double)RAND_MAX * 2.0) - 1.0;
    const double y = (rand()/(double)RAND_MAX * 2.0) - 1.0;
    if ( x*x + y*y < 1.0 ) {
      n_inside++;
    }
  }
  return n_inside;
}

int main( int argc, char *argv[] )
{
  int my_rank, comm_sz;
  int inside = 0, npoints = 1000000;
  double pi_approx;

  MPI_Init( &argc, &argv );	
  MPI_Comm_rank( MPI_COMM_WORLD, &my_rank );
  MPI_Comm_size( MPI_COMM_WORLD, &comm_sz );

  if ( argc > 1 ) {
    npoints = atoi(argv[1]);
  }

  /* Each process initializes the pseudo-random number generator; if
     we don't do this (or something similar), each process would
     produce the exact same sequence of pseudo-random numbers! */
  srand(time(NULL));

  /* [TODO] This is not a true parallel version; the master does
     everything */

  if (0 == my_rank) {
    const int start = npoints * my_rank / comm_sz;
    const int end = (my_rank + 1) * npoints / comm_sz;
    const int size = end - start;
    inside = generate_points(size);
    for (int p = 1; p < comm_sz; p++) {
      int remote_inside;
      MPI_Recv(&remote_inside, 1, MPI_INT, p, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      inside += remote_inside;
    }
    pi_approx = 4.0 * inside / (double)npoints;
    printf("PI approximation is %f (true value=%f, rel error=%.3f%%)\n", pi_approx, M_PI, 100.0*fabs(pi_approx-M_PI)/M_PI);

  } else {
    const int start = npoints * my_rank / comm_sz;
    const int end = (my_rank + 1) * npoints / comm_sz;
    const int size = end - start;
    int local_inside = generate_points(size);
    printf("Proc %d calculate\n", my_rank);
    MPI_Send(&local_inside, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
  }

  //if ( 0 == my_rank ) {
  //  inside = generate_points(npoints);
  //  pi_approx = 4.0 * inside / (double)npoints;
  //  printf("PI approximation is %f (true value=%f, rel error=%.3f%%)\n", pi_approx, M_PI, 100.0*fabs(pi_approx-M_PI)/M_PI);
  //}

  MPI_Finalize();
  return 0;
}

// vim: set nofoldenable :
