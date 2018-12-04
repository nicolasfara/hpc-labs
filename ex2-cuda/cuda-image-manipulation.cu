/* */
/****************************************************************************
 *
 * cuda-image-manipulation.c - Image manipulation with CUDA
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
 * ---------------------------------------------------------------------------
 *
 * Compile with:
 *
 * nvcc cuda-image-manipulation.cu -o cuda-image-manipulation
 *
 * Run with:
 *
 * ./cuda-image-manipulation < input_file > output_file
 *
 ****************************************************************************/
#include "hpc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLKSIZE 32

typedef struct {
  int width;   /* Width of the image (in pixels) */
  int height;  /* Height of the image (in pixels) */
  int maxgrey; /* Don't care (used only by the PGM read/write routines) */
  unsigned char *bmap; /* buffer of width*height bytes; each byte represents a pixel */
} img_t;

/**
 * Read a PGM file from file |f|. This function is not very robust; it
 * may fail on perfectly legal PGM images.
 */
void read_pgm( FILE *f, img_t* img )
{
  char buf[1024];
  const size_t BUFSIZE = sizeof(buf);
  char *s; 
  int nread;

  /* Get the file type (must be "P5") */
  s = fgets(buf, BUFSIZE, f);
  if (0 != strcmp(s, "P5\n")) {
    fprintf(stderr, "FATAL: wrong file type %s\n", buf);
    exit(EXIT_FAILURE);
  }
  /* Get any comment and ignore it; does not work if there are
     leading spaces in the comment line */
  do {
    s = fgets(buf, BUFSIZE, f);
  } while (s[0] == '#');
  sscanf(s, "%d %d", &(img->width), &(img->height));
  /* get maxgrey; must be less than or equal to 255 */
  s = fgets(buf, BUFSIZE, f);
  sscanf(s, "%d", &(img->maxgrey));
  if ( img->maxgrey > 255 ) {
    fprintf(stderr, "FATAL: maxgray > 255 (%d)\n", img->maxgrey);
    exit(EXIT_FAILURE);
  }
  /* Get the binary data */
  img->bmap = (unsigned char*)malloc((img->width)*(img->height));
  nread = fread(img->bmap, 1, (img->width)*(img->height), f);
  if ( (img->width)*(img->height) != nread ) {
    fprintf(stderr, "FATAL: error reading input: expecting %d bytes, got %d\n", (img->width)*(img->height), nread);
    exit(EXIT_FAILURE);
  }
}

/**
 * Write image |img| to file |f|
 */
void write_pgm( FILE *f, const img_t* img )
{
  fprintf(f, "P5\n");
  fprintf(f, "# produced by cuda-image-manipulation\n");
  fprintf(f, "%d %d\n", img->width, img->height);
  fprintf(f, "%d\n", img->maxgrey);
  fwrite(img->bmap, 1, (img->width)*(img->height), f);
}

/**
 * Free bitmap and set fields to invalid values
 */
void free_pgm( img_t *img )
{
  img->width = img->height = img->maxgrey = -1;
  free(img->bmap);
  img->bmap = NULL;
}

/**
 * Rotate image |orig| of size nxn 90 degrees clockwise; new image
 * goes to |rotated|
 */
__global__ void rotate_clockwise( unsigned char *orig, unsigned char *rotated, int n )
{
  /* [TODO] Implement this kernel */
  const int x = threadIdx.x + blockIdx.x * blockDim.x;
  const int y = threadIdx.y + blockIdx.y * blockDim.y;


  if (x < n && y < n) {
    rotated[(n - x - 1) * n + y] = orig[x * n + y];
    rotated[x * n + y] = orig[(n - x - 1) * n + y];
  }
}

/**
 * Flip image |orig| vertically; new image goes to |flipped|
 */
__global__ void vertical_flip( unsigned char *orig, unsigned char *flipped, int n )
{
  /* [TODO] Implement this kernel */
  const int x = threadIdx.x + blockIdx.x * blockDim.x;
  const int y = threadIdx.y + blockIdx.y * blockDim.y;


  if (x < n && y < n) {
    flipped[(n - x - 1) * n + y] = orig[x * n + y];
    flipped[x * n + y] = orig[(n - x - 1) * n + y];
  }
}

/**
 * Flip image |orig| horizontally; new image goes to |flipped|
 */
__global__ void horizontal_flip( unsigned char *orig, unsigned char *flipped, int n )
{
  /* [TODO] Implement this kernel */
  const int x = threadIdx.x + blockIdx.x * blockDim.x;
  const int y = threadIdx.y + blockIdx.y * blockDim.y;


  if (x < n && y < n) {
    flipped[(n - y - 1) * n + x] = orig[y * n + x];
    flipped[y * n + x] = orig[(n - y - 1) * n + x];
  }
}

int main( int argc, char* argv[] )
{
  img_t bmap;
  unsigned char *d_orig, *d_new, *tmp;
  double tstart, elapsed;
  int op;
  enum {
    OP_ROTATE = 1,
    OP_V_FLIP = 2,
    OP_H_FLIP = 4
  };

  if ( argc != 2 ) {
    fprintf(stderr, "Usage: %s op < input_file\nop = %d (rotate clockwise), %d (vertical flip), %d (horizontal flip)", argv[0], OP_ROTATE, OP_V_FLIP, OP_H_FLIP);
    return EXIT_FAILURE;
  }
  op = atoi(argv[1]);
  read_pgm(stdin, &bmap);
  if ( bmap.width != bmap.height ) {
    fprintf(stderr, "FATAL: width (%d) and height (%d) of the input image must be equal\n", bmap.width, bmap.height);
    return EXIT_FAILURE;
  }
  const int n = bmap.width;

  /* Allocate images on device */
  const size_t size = n*n;
  CudaSafeCall( cudaMalloc((void **)&d_orig, size) );
  CudaSafeCall( cudaMalloc((void **)&d_new, size) );

  /* Copy input to device */
  CudaSafeCall( cudaMemcpy(d_orig, bmap.bmap, size, cudaMemcpyHostToDevice) );
  const dim3 block(BLKSIZE, BLKSIZE);
  const dim3 grid((n + BLKSIZE - 1)/BLKSIZE, (n + BLKSIZE - 1)/BLKSIZE);

  tstart = hpc_gettime();
  if (op & OP_ROTATE) {
    fprintf(stderr, "Select 1\n");
    rotate_clockwise<<< grid, block >>>( d_orig, d_new, n);
    CudaCheckError();
    tmp = d_orig; d_orig = d_new; d_new = tmp;
  }
  if (op & OP_V_FLIP) {
    fprintf(stderr, "Select 2\n");
    vertical_flip<<< grid, block >>>( d_orig, d_new, n);
    CudaCheckError();
    tmp = d_orig; d_orig = d_new; d_new = tmp;
  }
  if (op & OP_H_FLIP) {
    fprintf(stderr, "Select 4\n");
    horizontal_flip<<< grid, block >>>( d_orig, d_new, n);
    CudaCheckError();
    tmp = d_orig; d_orig = d_new; d_new = tmp;
  }

  cudaDeviceSynchronize();
  elapsed = hpc_gettime() - tstart;
  /* Copy output to host */
  cudaMemcpy(bmap.bmap, d_orig, size, cudaMemcpyDeviceToHost);
  fprintf(stderr, "Execution time: %f\n", elapsed);
  write_pgm(stdout, &bmap);
  free_pgm(&bmap);
  cudaFree(d_orig);
  cudaFree(d_new);
  return EXIT_SUCCESS;
}
