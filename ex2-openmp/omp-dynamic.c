/* */
/****************************************************************************
 *
 * omp-dynamic.c - simulate "schedule(dynamic)" using the "omp parallel" clause
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
 * This program generates a list of n integers vin[0], ..., vin[n-1],
 * and compute the Fibonacci numbers Fib(vin[0]), ..., Fin(vin[n-1]).
 *
 * Compile with:
 *
 * gcc -std=c99 -Wall -Wpedantic -fopenmp omp-dynamic.c -o omp-dynamic
 *
 * Run with:
 *
 * ./omp-dynamic [n]
 *
 * Example:
 *
 * OMP_NUM_THREADS=2 ./omp_dynamic
 *
 ****************************************************************************/
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

/* Recursive computation of the n-th Fibonacci number, for n=0, 1, 2, ...
   Do not parallelize this function. */
int fib_rec(int n)
{
  if (n<2) {
    return 1;
  } else {
    return fib_rec(n-1) + fib_rec(n-2);
  }
}

/* Iterative computation of the n-th Fibonacci number. This function
   must be used for checking the result only. */
int fib_iter(int n)
{
  if (n<2) {
    return 1;
  } else {
    int fibnm1 = 1;
    int fibnm2 = 1;
    int fibn;
    n = n-1;
    do {
      fibn = fibnm1 + fibnm2;
      fibnm2 = fibnm1;
      fibnm1 = fibn;
      n--;
    } while (n>0);
    return fibn;        
  }
}

/* Initialize the content of vector v using the values from vstart to
   vend.  The vector is filled in such a way that there are more or
   less the same number of contiguous occurrences of all values in
   [vstart, vend]. */
void fill(int *v, int n)
{
  const int vstart = 20, vend = 35;
  const int blk = (n + vend - vstart) / (vend - vstart + 1);
  int tmp = vstart;

  for (int i=0; i<n; i+=blk) {
    for (int j=0; j<blk && i+j<n; j++) {
      v[i+j] = tmp;
    }
    tmp++;
  }
}

int main( int argc, char* argv[] )
{
  int i, n = 1024;
  const int max_n = 512*1024*1024;
  int *vin, *vout;

  if ( argc > 2 ) {
    fprintf(stderr, "Usage: %s [n]\n", argv[0]);
    return EXIT_FAILURE;
  }

  if ( argc > 1 ) {
    n = atoi(argv[1]);
  }

  if ( n > max_n ) {
    fprintf(stderr, "FATAL: n too large\n");
    return EXIT_FAILURE;
  }

  /* initialize the input and output arrays */
  vin = (int*)malloc(n * sizeof(vin[0]));
  vout = (int*)malloc(n * sizeof(vout[0]));

  /* fill input array */
  for (i=0; i<n; i++) {
    vin[i] = 25 + (i%10);
  }

  const double tstart = omp_get_wtime();

  /* [TODO] parallelize the following loop, simulating a
     "schedule(dynamic,1)" clause, i.e., dynamic scheduling with
     block size 1. Do not modify the body of the fib_rec()
     function. */
  int idx = 0;
#pragma omp parallel shared(idx)
  {
    int my_idx;
    do {
#pragma omp critical
      {
        my_idx = idx;
        idx++;
      }
      if (my_idx < n){
        vout[my_idx] = fib_rec(vin[my_idx]);
        printf("vin[%d]=%d vout[%d]=%d\n", my_idx, vin[my_idx], i, vout[my_idx]);
      }
    } while(my_idx < n);
  }

  const double elapsed = omp_get_wtime() - tstart;

  /* check result */
  for (i=0; i<n; i++) {
    if ( vout[i] != fib_iter(vin[i]) ) {
      fprintf(stderr,
          "Test FAILED: vin[%d]=%d, vout[%d]=%d (expected %d)\n",
          i, vin[i], i, vout[i], fib_iter(vin[i]));
      return EXIT_FAILURE;
    }
  }
  printf("Test OK\n");
  printf("Elapsed time: %f\n", elapsed);

  free(vin);
  free(vout);
  return EXIT_SUCCESS;
}

// vim: set nofoldenable :
