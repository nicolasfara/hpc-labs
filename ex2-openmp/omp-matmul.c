/* */
/****************************************************************************
 *
 * omp-matmul.c - Dense matrix-matrix multiply 
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
 * gcc -fopenmp omp-matmul.c -o omp-matmul
 *
 * Run with:
 * ./omp-matmul [n]
 *
 ****************************************************************************/
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/* Fills n x n square matrix m with random values */
void fill( double* m, int n )
{
  int i, j;
  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) {
      m[i*n + j] = (double)rand() / RAND_MAX;
    }
  }
}

int min(int a, int b)
{
  return (a < b ? a : b);
}

/**
 * Cache-efficient computation of r = p * q, where p. q, r are n x n
 * matrices. The caller is responsible for allocating the memory for
 * r. This function allocates (and the frees) an additional n x n
 * temporary matrix. 
 */
void matmul_transpose( double *p, double* q, double *r, int n)
{
  int i, j, k;
  double *qT = (double*)malloc( n * n * sizeof(*qT) );

  /* transpose q, storing the result in qT */
  /* [TODO] Parallelize the following loop(s) */

#pragma omp parallel for collapse(2) private(j)
  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) {
      qT[j*n + i] = q[i*n + j];
    }
  }

  /* multiply p and qT row-wise */
  /* [TODO] Parallelize the following loop(s) */
#pragma omp parallel for schedule(runtime) private(j) collapse(2)
  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) {
      double v = 0.0;
#pragma omp parallel for schedule(runtime) private(k) reduction(+:v)
      for (k=0; k<n; k++) {
        v += p[i*n + k] * qT[j*n + k];
      }
      r[i*n + j] = v;
    }
  }

  free(qT);
}

int main( int argc, char *argv[] )
{
  int n = 1000;
  const int max_n = 2000;
  double *p, *q, *r;

  if ( argc > 2 ) {
    fprintf(stderr, "Usage: %s [n]\n", argv[0]);
    return EXIT_FAILURE;
  }

  if ( argc > 1 ) {
    n = atoi(argv[1]);
  }

  if ( n > max_n ) {
    fprintf(stderr, "FATAL: matrix size is too large\n");
    return EXIT_FAILURE;
  }

  printf("Matrix-matrix multiply (%d x %d)...\n", n, n);
  const size_t size = n*n*sizeof(double);

  p = (double*)malloc( size ); assert(p);
  q = (double*)malloc( size ); assert(q);
  r = (double*)malloc( size ); assert(r);

  fill(p, n);
  fill(q, n);

  const double tstart = omp_get_wtime();
  matmul_transpose(p, q, r, n);
  const double elapsed = omp_get_wtime() - tstart;
  printf("Done\nElapsed time: %f\n", elapsed);

  free(p);
  free(q);
  free(r);

  return EXIT_SUCCESS;
}

// vim: set nofoldenable :
