/* */
/****************************************************************************
 *
 * mpi-dot.c - Parallel dot product using MPI.
 *
 * Written in 2016 by Moreno Marzolla <moreno.marzolla(at)unibo.it>
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
 * Compile with:
 * mpicc -std=c99 -Wall -Wpedantic mpi-dot.c -lm -o mpi-dot
 *
 * Run with:
 * mpirun -n 4 ./mpi-dot
 *
 ****************************************************************************/
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h> /* for fabs() */

/*
 * Compute sum { x[i] * y[i] }, i=0, ... n-1
 */
double dot( const double* x, const double* y, int n )
{
  double s = 0.0;
  int i;
  for (i=0; i<n; i++) {
    s += x[i] * y[i];
  }
  return s;
}

int main(int argc, char* argv[])
{
  double *x = NULL, *y = NULL, result = 0.0;
  int n = 1000;
  int my_rank, comm_sz;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

  if (argc > 1) {
    n = atoi(argv[1]);
  }

  /* [TODO] This is not a true parallel version, since the master
     does everything */
  if (0 == my_rank) {
    /* The master allocates the vectors */
    int i;
    x = (double*) malloc(n * sizeof(*x));
    y = (double*) malloc(n * sizeof(*y));
    for (i=0; i < n; i++) {
      x[i] = i + 1.0;
      y[i] = 1.0 / x[i];
    }
  }

  int *sendCounts = NULL, *displs = NULL;
  double *localx, *localy;
  sendCounts = (int*) malloc(comm_sz * sizeof(int));
  displs = (int*) malloc(comm_sz * sizeof(int));
  for (int i = 0; i < comm_sz; i++) {
    const int start = n * i / comm_sz;
    const int end = n * (i + 1) / comm_sz;
    const int size = end - start;
    sendCounts[i] = size;
    displs[i] = start;
  }

  const int local_n = sendCounts[my_rank];
  localx = (double*) malloc(local_n * sizeof(double));
  localy = (double*) malloc(local_n * sizeof(double));

  MPI_Scatterv(x, sendCounts, displs, MPI_DOUBLE, localx, local_n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  MPI_Scatterv(y, sendCounts, displs, MPI_DOUBLE, localy, local_n, MPI_DOUBLE, 0, MPI_COMM_WORLD);

  double local_result = dot(localx, localy, local_n);

  MPI_Reduce(&local_result, &result, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

  if (0 == my_rank) {
    printf("Dot product: %f\n", result);
    if (fabs(result - n) < 1e-5) {
      printf("Check OK\n");
    } else {
      printf("Check failed: got %f, expected %f\n", result, (double)n);
    }
  }

  free(x); /* if x == NULL, does nothing */
  free(y);
  free(localx);
  free(localy);
  free(sendCounts);
  free(displs);

  MPI_Finalize();

  return 0;
}


// vim: set nofoldenable :
