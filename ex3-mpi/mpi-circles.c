/* */
/****************************************************************************
 *
 * mpi-circles.c - Monte Carlo computation of the area of the union of a set of circles
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
 * mpicc -std=c99 -Wall -Wpedantic mpi-circles.c -o mpi-circles
 *
 * Run with:
 * mpirun -n 4 ./mpi-circles 10000 circles-1000.in
 *
 ****************************************************************************/

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h> /* for time() */

float sq(float x)
{
  return x*x;
}

/* Generate |k| random points inside the square (0,0) --
   (100,100). Return the number of points that fall inside at least one
   of the |n| circles with center (x[i], y[i]) and radius r[i].  The
   result must be <= |k|. */
int inside( const float* x, const float* y, const float *r, int n, int k )
{
  int i, np, c=0;
  for (np=0; np<k; np++) {
    const float px = 100.0*rand()/(float)RAND_MAX;
    const float py = 100.0*rand()/(float)RAND_MAX;
    for (i=0; i<n; i++) {
      if ( sq(px-x[i]) + sq(py-y[i]) <= sq(r[i]) ) {
        c++;
        break;
      }
    }
  }
  return c;
}

int main( int argc, char* argv[] )
{
  float *x = NULL, *y = NULL, *r = NULL;
  int N, K, c = 0;
  int my_rank, comm_sz;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

  /* Initialize the Random Number Generator (RNG) */
  srand(time(NULL));

  if ( (0 == my_rank) && (argc != 3) ) {
    fprintf(stderr, "Usage: %s [npoints] [inputfile]\n", argv[0]);
    MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
  }

  K = atoi(argv[1]);

  /* It is required that the input file is read by the master only */
  if ( 0 == my_rank ) {
    FILE *in = fopen(argv[2], "r");
    int i;
    if ( !in ) {
      fprintf(stderr, "Cannot open %s for reading\n", argv[2]);
      MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }
    fscanf(in, "%d", &N);
    x = (float*)malloc(N * sizeof(*x));
    y = (float*)malloc(N * sizeof(*y));
    r = (float*)malloc(N * sizeof(*r));
    for (i=0; i<N; i++) {
      fscanf(in, "%f %f %f", &x[i], &y[i], &r[i]);
    }
    fclose(in);
  }

  MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);

  const int start = my_rank * K / comm_sz;
  const int end = (my_rank + 1) * K / comm_sz;
  const int size = end - start;

  if(my_rank != 0) {
    x = (float*) malloc(N * sizeof(*x));
    y = (float*) malloc(N * sizeof(*y));
    r = (float*) malloc(N * sizeof(*r));
  }

  MPI_Bcast(x, N, MPI_FLOAT, 0, MPI_COMM_WORLD);
  MPI_Bcast(y, N, MPI_FLOAT, 0, MPI_COMM_WORLD);
  MPI_Bcast(r, N, MPI_FLOAT, 0, MPI_COMM_WORLD);

  int local_c = inside(x, y, r, N, size);

  MPI_Reduce(&local_c, &c, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

  /* the master prints the area */
  if ( 0 == my_rank ) {
    printf("%d points, %d inside, area %f\n", K, c, 1.0e6*c/K);
  }

  free(x);
  free(y);
  free(r);

  MPI_Finalize();

  return EXIT_SUCCESS;
}


// vim: set nofoldenable :
