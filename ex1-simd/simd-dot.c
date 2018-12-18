/* */
/****************************************************************************
 *
 * simd-dot,c - Dot product using SIMD extensions
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
 * ----------------------------------------------------------------------------
 *
 * Compile with:
 * gcc -std=c99 -Wall -Wpedantic -D_XOPEN_SOURCE=600 -O2 -march=native simd-dot.c -lm -o simd-dot
 *
 * Run with:
 * ./simd-dot [n]
 *
 * Example:
 * ./simd-dot 20000000
 *
 ******************************************************************************/

/* The following #define is required by posix_memalign() */
#define _XOPEN_SOURCE 600

#include "hpc.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <strings.h> /* for bzero() */
#include <math.h> /* for fabs() */

typedef float v4f __attribute__((vector_size(16)));
#define VLEN (sizeof(v4f)/sizeof(float))

/* Returns the minimum value of a nonempty array v with n elements */
float serial_dot(const float *x, const float *y, int n)
{
  double r = 0.0; /* use double here to avoid some nasty rounding errors */
  int i;
  for (i=0; i<n; i++) {
    r += x[i] * y[i];
  }
  return r;
}

/* Same as above, but using the vector datatype of GCC */
float simd_dot(const float *x, const float *y, int n)
{
  v4f *m_x, *m_y;
  v4f vs = { 0.0f, 0.0f, 0.0f, 0.0f };
  float sum = 0.0;
  int i;
  for (i = 0; i < n - VLEN + 1; i += VLEN) {
    m_x = (v4f *) (x + i);
    m_y = (v4f *) (y + i);
    vs += *m_x * *m_y;
  }

  sum = vs[0] + vs[1] + vs[2] + vs[3];
  for (; i < n; i++) {
    sum += x[i];
  }

  return sum;
}

/* Initialize vectors x and y */
void fill(float* x, float* y, int n)
{
  int i;
  const float xx[] = {-2.0f, 0.0f, 4.0f, 2.0f};
  const float yy[] = { 1.0f/2.0, 0.0f, 1.0/16.0, 1.0f/2.0f};

  for (i=0; i<n; i++) {
    x[i] = xx[i % sizeof(xx)/sizeof(xx[0])];
    y[i] = yy[i % sizeof(yy)/sizeof(yy[0])];;
  }
}

int main(int argc, char* argv[])
{
  const int nruns = 10; /* number of replications */
  int r, n = 10*1024*1024;
  double serial_elapsed, simd_elapsed;
  double tstart, tend;
  float *x, *y, serial_result, simd_result;
  int ret;

  if ( argc > 2 ) {
    fprintf(stderr, "Usage: %s [n]\n", argv[0]);
    return EXIT_FAILURE;
  }

  if ( argc > 1 ) {
    n = atoi(argv[1]);
  }

  assert(n > 0);

  const size_t size = n * sizeof(*x);

  assert( size < 1024*1024*200UL );
  ret = posix_memalign((void**)&x, __BIGGEST_ALIGNMENT__, size); 
  assert( 0 == ret );
  ret = posix_memalign((void**)&y, __BIGGEST_ALIGNMENT__, size); 
  assert( 0 == ret );

  printf("Array length = %d\n", n);

  fill(x, y, n);
  /* Collect execution time of serial version */
  serial_elapsed = 0.0;
  for (r=0; r<nruns; r++) {
    tstart = hpc_gettime();
    serial_result = serial_dot(x, y, n);
    tend = hpc_gettime();
    serial_elapsed += tend - tstart;
  }
  serial_elapsed /= nruns;

  fill(x, y, n);
  /* Collect execution time of the parallel version */
  simd_elapsed = 0.0;
  for (r=0; r<nruns; r++) {
    tstart = hpc_gettime();
    simd_result = simd_dot(x, y, n);
    tend = hpc_gettime();
    simd_elapsed += tend - tstart;
  }
  simd_elapsed /= nruns;

  printf("Serial: result=%f, time=%f (%d runs)\n", serial_result, serial_elapsed, nruns);
  printf("SIMD  : result=%f, time=%f (%d runs)\n", simd_result, simd_elapsed, nruns);

  if ( fabs(serial_result - simd_result) > 1e-5 ) {
    fprintf(stderr, "Check FAILED\n");
    return EXIT_FAILURE;
  }

  printf("Speedup (serial/simd) %f\n", serial_elapsed / simd_elapsed);

  free(x);
  free(y);
  return EXIT_SUCCESS;
}

// vim: set nofoldenable :
