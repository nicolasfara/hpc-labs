/* */
/****************************************************************************
 *
 * omp-sieve.c - Parallel implementation of the Sieve of Eratosthenes
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
 * This program counts the prime numbers in the set {2, ..., n} using
 * the sieve of Eratosthenes.
 *
 * Compile with:
 *
 * gcc -std=c99 -Wall -Wpedantic -fopenmp omp-sieve.c -o omp-sieve
 *
 * Run with:
 *
 * ./omp-sieve [n]
 *
 * You should expect the following results:
 *
 *                     num. of primes
 *                n     in {2, ... n}
 *      -----------    --------------
 *                1                 0
 *               10                 4
 *              100                25
 *            1,000               168
 *           10,000             1,229
 *          100,000             9,592
 *        1,000,000            78,498
 *       10,000,000           664,579
 *      100,000,000         5,761,455
 *    1,000,000,000        50,847,534
 *   10,000,000,000                 ?
 *
 ****************************************************************************/
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h> /* for memset() */

/* Mark all mutliples of |p| in the set {from, ..., to-1}; return how
   many numbers have been marked for the first time. |from| does not
   need to be a multiple of |p|. */
long mark( char *isprime, long from, long to, long p )
{
  long nmarked = 0l;
  /* [TODO] Parallelize this function */
  from = ((from + p - 1)/p)*p; /* start from the lowest multiple of p that is >= from */
#pragma omp parallel for reduction(+:nmarked) default(none) shared(from, to, p, isprime)
  for ( long x=from; x<to; x+=p ) {
    if (isprime[x]) {
      isprime[x] = 0;
      nmarked++;
    }
  }
  return nmarked;
}

int main( int argc, char *argv[] )
{
  long n = 1000000l, nprimes;
  char *isprime;

  if ( argc > 2 ) {
    fprintf(stderr, "Usage: %s [n]\n", argv[0]);
    return EXIT_FAILURE;
  }

  if ( argc == 2 ) {
    n = atol(argv[1]);
  }

  if (n > (1ul << 31)) {
    fprintf(stderr, "FATAL: n too large\n");
    return EXIT_FAILURE;
  }

  isprime = (char*)malloc(n+1); assert(isprime);
  memset(isprime, 1, n+1);
  const double tstart = omp_get_wtime();
  /* Initialize isprime[] to 1 */
  nprimes = n-1;
  /* main iteration of the sieve */
  for (long i=2; i*i <= n; i++) {
    if (isprime[i]) {
      nprimes -= mark(isprime, i*i, n+1, i);
    }
  }
  const double elapsed = omp_get_wtime() - tstart;
  /*
     for (long i=2; i<=n; i++) {
     if (isprime[i]) {printf("%ld ", i);}
     }
     printf("\n");
     */
  free(isprime);
  printf("There are %ld primes in {2, ..., %ld}\n", nprimes, n);
  printf("Elapsed time: %f\n", elapsed);
  return EXIT_SUCCESS;
}

// vim: set nofoldenable :
