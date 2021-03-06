/* */
/****************************************************************************
 *
 * cuda-dot.cu - Dot product with CUDA
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
 * ---------------------------------------------------------------------------
 *
 * Compile with:
 * nvcc cuda-dot.cu -o cuda-dot -lm
 *
 * Run with:
 * ./cuda-dot [len]
 *
 * Example:
 * ./cuda-dot
 *
 ****************************************************************************/
#include "hpc.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <thrust/reduce.h>
#include <thrust/functional.h>

#define N 1024*1024


__global__ void dot( double *tmp, double *x, double *y, int n )
{
  /* [TODO] modify this function so that (part of) the dot product
     computation is executed on the GPU. */
  //double result = 0.0;
  //for (int i = 0; i < n; i++) {
  //    result += x[i] * y[i];
  //}
  //return result;
  int index = threadIdx.x + blockIdx.x * blockDim.x;
  if (index < n) {
    tmp[index] = x[index] * y[index];
  }
}

void vec_init( double *x, double *y, int n )
{
  int i;
  const double tx[] = {1.0/64.0, 1.0/128.0, 1.0/256.0};
  const double ty[] = {1.0, 2.0, 4.0};
  const size_t arrlen = sizeof(tx)/sizeof(tx[0]);

  for (i=0; i<n; i++) {
    x[i] = tx[i % arrlen];
    y[i] = ty[i % arrlen];
  }
}

int main( int argc, char* argv[] )
{
  double *tmp, *x, *y, result;
  double *_tmp, *_x, *_y;
  int n = 1024*1024;
  const int max_len = 128 * n;
  const int blksize = 640;

  if ( argc > 2 ) {
    fprintf(stderr, "Usage: %s [len]\n", argv[0]);
    return EXIT_FAILURE;
  }

  if ( argc > 1 ) {
    n = atoi(argv[1]);
  }

  if ( n > max_len ) {
    fprintf(stderr, "FATAL: the maximum length is %d\n", max_len);
    return EXIT_FAILURE;
  }

  const size_t size = n*sizeof(*x);

  /* Allocate space for host copies of x, y */
  x = (double*)malloc(size); assert(x);
  y = (double*)malloc(size); assert(y);
  tmp = (double*)malloc(size); assert(tmp);
  vec_init(x, y, n);

  cudaError err = cudaMalloc((void **)&_x, size);
  assert(err == cudaSuccess);
  err = cudaMalloc((void **)&_y, size);
  assert(err == cudaSuccess);
  err = cudaMalloc((void **)&_tmp, size);
  assert(err == cudaSuccess);

  err = cudaMemcpy(_x, x, size, cudaMemcpyHostToDevice);
  assert(err == cudaSuccess);
  err = cudaMemcpy(_y, y, size, cudaMemcpyHostToDevice);
  assert(err == cudaSuccess);

  double start = hpc_gettime();

  printf("Computing the dot product of %d elements... ", n);
  dot<<<(n + blksize - 1) / blksize, blksize>>>(_tmp, _x, _y, n);
  CudaCheckError();
  cudaDeviceSynchronize();

  cudaMemcpy(tmp, _tmp, size, cudaMemcpyDeviceToHost);
  //for (int i = 0; i < n; i++) {
  //  result += tmp[i];
  //}

  result = thrust::reduce(tmp, tmp + n, 0.0, thrust::plus<double>());

  double stop = hpc_gettime();

  cudaFree(_x);
  cudaFree(_y);
  cudaFree(_tmp);

  printf("result=%f\n", result);

  const double expected = ((double)n)/64;

  /* Check result */
  if ( fabs(result - expected) < 1e-5 ) {
    printf("Check OK\n");
  } else {
    printf("Check FAILED: got %f, expected %f\n", result, expected);
  }

  printf("Time: %f", stop-start);

  /* Cleanup */
  free(x); free(y);

  return EXIT_SUCCESS;
}
