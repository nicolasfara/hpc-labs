/* */
/****************************************************************************
 *
 * cuda-rule30.cu - Rule30 Cellular Automaton with CUDA
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
 * This program implements the "rule 30 CA" as described in
 * https://en.wikipedia.org/wiki/Rule_30 . This program uses the CPU
 * only; the task is to parallelize the computation so that the GPU is
 * used.
 *
 * Compile with:
 * nvcc cuda-rule30.cu -o cuda-rule30
 *
 * Run with:
 * /cuda-rule30 1024 1024
 *
 ****************************************************************************/

#include "hpc.h"
#include <stdio.h>
#include <stdlib.h>


#define BLKSIZE 1024

typedef unsigned char cell_t;

/**
 * Given the current state of the CA, compute the next state.  This
 * version requires that the |cur| and |next| arrays are extended with
 * ghost cells; therefore, |ext_n| is the length of |cur| and |next|
 * _including_ ghost cells.
 *
 *                             +----- ext_n-2
 *                             |   +- ext_n-1
 *   0   1                     V   V
 * +---+-------------------------+---+
 * |///|                         |///|
 * +---+-------------------------+---+
 *
 */

__global__ void step_kernel(cell_t *cur, cell_t *next, int ext_n)
{
  __shared__ cell_t buf[BLKSIZE + 2];
  const int gi = threadIdx.x + blockIdx.x * blockDim.x + 1;
  const int li = threadIdx.x + 1;
  if (gi < ext_n - 1) {
    buf[li] = cur[gi];
    if (li == 1) {
      buf[0] = cur[gi - 1];
      buf[BLKSIZE + 1] = cur[gi + BLKSIZE];
    }

    __syncthreads();

    const cell_t left   = buf[li-1];
    const cell_t center = buf[li  ];
    const cell_t right  = buf[li+1];
    next[gi] =
      (left && !center && !right) ||
      (!left && !center &&  right) ||
      (!left &&  center && !right) ||
      (!left &&  center &&  right);
  }
}

__global__ void fill_ghost(cell_t *d_cur, int ext_n)
{
  d_cur[ext_n - 1] = d_cur[1];
  d_cur[0] = d_cur[ext_n - 2];
}

/**
 * Initialize the domain; all cells are 0, with the exception of a
 * single cell in the middle of the domain. |cur| points to an array
 * of length |ext_n|; the length includes two ghost cells.
 */
void init_domain( cell_t *cur, int ext_n )
{
  int i;
  for (i=0; i<ext_n; i++) {
    cur[i] = 0;
  }
  cur[ext_n/2] = 1;
}

/**
 * Dump the current state of the CA to PBM file |out|. |cur| points to
 * an array of length |ext_n| that includes two ghost cells.
 */
void dump_state( FILE *out, const cell_t *cur, int ext_n )
{
  int i;
  for (i=1; i < ext_n-1; i++) {
    fprintf(out, "%d ", cur[i]);
  }
  fprintf(out, "\n");
}

int main( int argc, char* argv[] )
{
  const char *outname = "rule30.pbm";
  FILE *out;
  int width = 1024, steps = 1024, s;
  cell_t *cur;
  cell_t *d_cur, *d_next;

  if ( argc > 3 ) {
    fprintf(stderr, "Usage: %s [width [steps]]\n", argv[0]);
    return EXIT_FAILURE;
  }

  if ( argc > 1 ) {
    width = atoi(argv[1]);
  }

  if ( argc > 2 ) {
    steps = atoi(argv[2]);
  }

  if (width % BLKSIZE) {
    fprintf(stderr, "width must be multiple of BLKSIZE\n");
    return EXIT_FAILURE;
  }

  const int ext_width = width + 2;
  const size_t ext_size = ext_width * sizeof(*cur); /* includes ghost cells */

  /* Create the output file */
  out = fopen(outname, "w");
  if ( !out ) {
    fprintf(stderr, "FATAL: cannot create file \"%s\"\n", outname);
    return EXIT_FAILURE;
  }
  fprintf(out, "P1\n");
  fprintf(out, "# produced by %s %d %d\n", argv[0], width, steps);
  fprintf(out, "%d %d\n", width, steps);

  /* Allocate space for the cur[] and next[] arrays */
  cur = (cell_t*)malloc(ext_size);
  CudaSafeCall(cudaMalloc((void **)&d_cur, ext_size));
  CudaSafeCall(cudaMalloc((void **)&d_next, ext_size));

  /* Initialize the domain */
  init_domain(cur, ext_width);

  CudaSafeCall(cudaMemcpy(d_cur, cur, ext_size, cudaMemcpyHostToDevice));

  /* Evolve the CA */
  for (s=0; s<steps; s++) {

    /* Dump the current state */
    dump_state(out, cur, ext_width);

    /* Fill ghost cells */
    /*
    cur[ext_width-1] = cur[1];
    cur[0] = cur[ext_width-2];
    */
    fill_ghost<<<1, 1>>>(d_cur, ext_width);
    CudaCheckError();

    /* Compute next state */
    step_kernel<<<(width + BLKSIZE - 1) / BLKSIZE, BLKSIZE>>>(d_cur, d_next, ext_width);
    CudaCheckError();

    CudaSafeCall(cudaMemcpy(cur, d_cur, ext_size, cudaMemcpyDeviceToHost));

    /* swap cur and next */
    cell_t *tmp = d_cur;
    d_cur = d_next;
    d_next = tmp;
  }

  free(cur);
  cudaFree(d_cur);
  cudaFree(d_next);

  fclose(out);

  return EXIT_SUCCESS;
}
