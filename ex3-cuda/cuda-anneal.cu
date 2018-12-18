/* */
/****************************************************************************
 *
 * cuda-anneal.cu - Annealing with CUDA
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
 * CUDA implementaiton of the "ANNEAL" cellular automaton, with and
 * without shared memory.
 *
 * Compile with:
 * nvcc cuda-anneal.cu -o cuda-anneal
 *
 * Run with:
 * ./cuda-anneal [steps [n]]
 *
 * Example:
 * ./cuda-anneal 64
 * produces a file anneal-00064.pbm
 *
 ****************************************************************************/
#include "hpc.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define BLKDIM_COPY 1024
#define BLKDIM      32


typedef unsigned char cell_t;

/* The following function makes indexing of the 2D domain
   easier. Instead of writing, e.g., grid[i*ext_n + j] you write
   IDX(grid, ext_n, i, j) to get a pointer to grid[i][j]. This
   function assumes that the size of the CA grid is (ext_n*ext_n),
   where the first and last rows/columns are ghost cells.

   Note the use of both the __device__ and __host__ qualifiers: this
   function can be called from host and device code. */
__device__ __host__ cell_t* IDX(cell_t *grid, int ext_n, int i, int j)
{
  return (grid + i*ext_n + j);
}

__device__ int d_min(int a, int b)
{
  return (a<b ? a : b);
}

/*
   |grid| points to a (ext_n * ext_n) block of bytes; this function
   copies the top and bottom ext_n elements to the opposite halo (see
   figure below).

   ext_n-2
   0 1              | ext_n-1
   | |              | |
   v v              v v
   +-+----------------+-+
   |Y|YYYYYYYYYYYYYYYY|Y| <- 0
   +-+----------------+-+
   |X|XXXXXXXXXXXXXXXX|X| <- 1
   | |                | |
   | |                | |
   | |                | |
   | |                | |
   |Y|YYYYYYYYYYYYYYYY|Y| <- ext_n - 2
   +-+----------------+-+
   |X|XXXXXXXXXXXXXXXX|X| <- ext_n - 1
   +-+----------------+-+

 */
/* [TODO] Transform this function into a kernel */
__global__ void copy_top_bottom(cell_t *grid, int ext_n)
{
  int j = threadIdx.x + blockIdx.x * blockDim.x;
  if (j < ext_n) {
    *IDX(grid, ext_n, ext_n-1,j) = *IDX(grid, ext_n, 1, j); /* top to bottom halo */
    *IDX(grid, ext_n, 0, j)= *IDX(grid, ext_n, ext_n-2, j); /* bottom to top halo */
  }
}

/*
   |grid| points to a ext_n*ext_n block of bytes; this function copies
   the left and right ext_n elements to the opposite halo (see figure
   below).

   ext_n-2
   0 1              | ext_n-1
   | |              | |
   v v              v v
   +-+----------------+-+
   |Y|X              Y|X| <- 0
   +-+----------------+-+
   |Y|X              Y|X| <- 1
   |Y|X              Y|X|
   |Y|X              Y|X|
   |Y|X              Y|X|
   |Y|X              Y|X|
   |Y|X              Y|X| <- ext_n - 2
   +-+----------------+-+
   |Y|X              Y|X| <- ext_n - 1
   +-+----------------+-+

 */
/* [TODO] This function should be transformed into a kernel */
__global__ void copy_left_right(cell_t *grid, int ext_n)
{
  const int i = threadIdx.x + blockIdx.x + blockDim.x;
  if (i < ext_n) {
    *IDX(grid, ext_n, i, ext_n-1) = *IDX(grid, ext_n, i, 1); /* left column to right halo */
    *IDX(grid, ext_n, i, 0) = *IDX(grid, ext_n, i, ext_n-2); /* right column to left halo */
  }
}

/* Compute the |next| grid given the current configuration |cur|.
   Both grids have (ext_n*ext_n) elements.
   [TODO] This function should be transformed into a kernel. */
__global__ void step(cell_t *cur, cell_t *next, int ext_n)
{
  __shared__ cell_t buf[BLKDIM][BLKDIM];

  const int gj = threadIdx.x + blockIdx.x * (blockDim.x - 2);
  const int gi = threadIdx.y + blockIdx.y * (blockDim.y - 2);

  const int li = threadIdx.y;
  const int lj = threadIdx.x;

  if (gi < ext_n && gj < ext_n) {
    buf[li][lj] = *IDX(cur, ext_n, gi, gj);
    __syncthreads();
    if (li > 0 && li < blockDim.y - 1 && lj > 0 && lj < blockDim.x - 1) {
      const int nbors =
        buf[li - 1][lj - 1] + buf [li - 1][lj] + buf[li - 1][lj + 1] +
        buf[li][lj - 1] + buf[li][lj] + buf[li][lj + 1] +
        buf[li + 1][lj - 1] + buf[li + 1][lj] + buf[li + 1][lj + 1];
        *IDX(next, ext_n, gi, gj) = (nbors >= 6 || nbors == 4);
    }
  }

}

/* Initialize the current grid |cur| with alive cells with density
   |p|. */
void init( cell_t *cur, int ext_n, float p )
{
  int i, j;
  for (i=1; i<ext_n-1; i++) {
    for (j=1; j<ext_n-1; j++) {
      *IDX(cur, ext_n, i, j) = (((float)rand())/RAND_MAX < p);
    }
  }
}

/* Write |cur| to file |fname| in pbm (portable bitmap) format. */
void write_pbm( cell_t *cur, int ext_n, const char* fname )
{
  int i, j;
  FILE *f = fopen(fname, "w");
  if (!f) { 
    fprintf(stderr, "Cannot open %s for writing\n", fname);
    exit(EXIT_FAILURE);
  }
  fprintf(f, "P1\n");
  fprintf(f, "# produced by cuda-anneal.cu\n");
  fprintf(f, "%d %d\n", ext_n-2, ext_n-2);
  for (i=1; i<ext_n-1; i++) {
    for (j=1; j<ext_n-1; j++) {
      fprintf(f, "%d ", *IDX(cur, ext_n, i, j));
    }
    fprintf(f, "\n");
  }
  fclose(f);
}

int main( int argc, char* argv[] )
{
  char fname[128];
  cell_t *cur, *next;
  cell_t *d_cur, *d_next;
  int s, nsteps = 64, n = 256;

  if ( argc > 3 ) {
    fprintf(stderr, "Usage: %s [nsteps [n]]\n", argv[0]);
    return EXIT_FAILURE;
  }

  if ( argc > 1 ) {
    nsteps = atoi(argv[1]);
  }

  if ( argc > 2 ) {
    n = atoi(argv[2]);
    n = (n > 2048 ? 2048 : n); /* maximum image size is 2048 */
  }

  const int ext_n = n+2;
  const size_t ext_size = ext_n * ext_n * sizeof(cell_t);

  fprintf(stderr, "Anneal CA: steps=%d size=%d\n", nsteps, n);
  cur = (cell_t*)malloc(ext_size); assert(cur);
  next = (cell_t*)malloc(ext_size); assert(next);

  cudaMalloc((void **)&d_cur, ext_size);
  cudaMalloc((void **)&d_next, ext_size);

  init(cur, ext_n, 0.5);

  cudaMemcpy(d_cur, cur, ext_size, cudaMemcpyHostToDevice);

  dim3 block(BLKDIM, BLKDIM);
  dim3 grid((n + BLKDIM - 3) / (BLKDIM - 2), (n + BLKDIM - 3) / (BLKDIM - 2));
  const double tstart = hpc_gettime();
  for (s=0; s<nsteps; s++) {
    copy_top_bottom<<<(ext_n + BLKDIM_COPY - 1) / BLKDIM_COPY, BLKDIM_COPY>>>(d_cur, ext_n);
    copy_left_right<<<(ext_n + BLKDIM_COPY - 1) / BLKDIM_COPY, BLKDIM_COPY>>>(d_cur, ext_n);
    step<<<grid, block>>>(d_cur, d_next, ext_n);
    cell_t *tmp = d_cur;
    d_cur = d_next;
    d_next = tmp;
  }
  cudaDeviceSynchronize();
  const double elapsed = hpc_gettime() - tstart;
  cudaMemcpy(cur, d_cur, ext_size, cudaMemcpyDeviceToHost);
  snprintf(fname, sizeof(fname), "anneal-%05d.pbm", s);
  write_pbm(cur, ext_n, fname);
  free(cur);
  free(next);
  cudaFree(d_cur);
  cudaFree(d_next);
  fprintf(stderr, "Execution time: %f\n", elapsed);

  return EXIT_SUCCESS;
}
