/* */
/****************************************************************************
 *
 * cuda-cat-map.cu - Arnold's cat map with CUDA
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
 * ---------------------------------------------------------------------------
 *
 * Compile with:
 *
 * nvcc cuda-cat-map.cu -o cuda-cat-map
 *
 * Run with:
 *
 * ./cuda-cat-map k < input_file > output_file
 *
 * to compute the k-th iterate of the cat map.  Input and output files
 * are in PGM (Portable Graymap) format; see "man pgm" for details
 * (however, you do not need to know anything about the PGM formato;
 * functions are provided below to read and write a PGM file). The
 * file cat.pgm can be used as a test image.  Example:
 *
 * ./cuda-cat-map 100 < cat.pgm > cat.100.pgm
 *
 * See https://en.wikipedia.org/wiki/Arnold%27s_cat_map for an explanation
 * of the cat map.
 *
 ****************************************************************************/
#include "hpc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define BLKSIZE 32


typedef struct {
  int width;   /* Width of the image (in pixels) */
  int height;  /* Height of the image (in pixels) */
  int maxgrey; /* Don't care (used only by the PGM read/write routines) */
  unsigned char *bmap; /* buffer of width*height bytes; each byte represents a pixel */
} img_t;

/**
 * Read a PGM file from file |f|. This function is not very robust; it
 * may fail on perfectly legal PGM images, but works for the provided
 * cat.pgm file.
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
  fprintf(f, "# produced by cuda-cat-map\n");
  fprintf(f, "%d %d\n", img->width, img->height);
  fprintf(f, "%d\n", img->maxgrey);
  fwrite(img->bmap, 1, (img->width)*(img->height), f);
}

/**
 * Free bitmap
 */
void free_pgm( img_t *img )
{
  img->width = img->height = img->maxgrey = -1;
  free(img->bmap);
  img->bmap = NULL;
}


/**
 * Compute the |k|-th iterate of the cat map for image |img|. The
 * width and height of the input image must be equal. This function
 * replaces the bitmap of |img| with the one resulting after ierating
 * |k| times the cat map. You need to allocate a temporary image, with
 * the same size of the original one, so that you read the pixel from
 * the "old" image and copy them to the "new" image (this is similar
 * to a stencil computation, as was discussed in class). After
 * applying the cat map to all pixel of the "old" image the role of
 * the two images is exchanged: the "new" image becomes the "old" one,
 * and vice-versa. At the end of the function, the temporary image
 * must be deallocated.
 */
__global__ void cat_map(unsigned char* cur, unsigned char *next, int N )
{

  /* [TODO] Modify the body of this function to allocate device memory,
     do the appropriate data transfer, and launch a kernel */

  const int x = threadIdx.x + blockIdx.x * blockDim.x;
  const int y = threadIdx.y + blockIdx.y * blockDim.y;

  if (x < N && y < N) {
    int xnext = (2*x+y) % N;
    int ynext = (x + y) % N;
    next[xnext + ynext*N] = cur[x+y*N];
  }
}

int main( int argc, char* argv[] )
{
  img_t bmap;
  unsigned char *cur, *next;
  int niter;

  if ( argc != 2 ) {
    fprintf(stderr, "Usage: %s niter < input_image > output_image\n", argv[0]);
    return EXIT_FAILURE;
  }
  niter = atoi(argv[1]);
  read_pgm(stdin, &bmap);
  if ( bmap.width != bmap.height ) {
    fprintf(stderr, "FATAL: width (%d) and height (%d) of the input image must be equal\n", bmap.width, bmap.height);
    return EXIT_FAILURE;
  }

  const int bytes = bmap.height * bmap.width;

  CudaSafeCall(cudaMalloc((void **)&cur, bytes));
  CudaSafeCall(cudaMalloc((void **)&next, bytes));
  CudaSafeCall(cudaMemcpy(cur, bmap.bmap, bytes, cudaMemcpyHostToDevice));

  const double tstart = hpc_gettime();
  dim3 blk((bmap.width + BLKSIZE - 1) / BLKSIZE, (bmap.width + BLKSIZE - 1) / BLKSIZE);
  dim3 thr(BLKSIZE, BLKSIZE);
  for (int i = 0; i < niter; i++) {
    cat_map<<<blk, thr>>>(cur, next, bmap.height);
    cudaDeviceSynchronize();
    unsigned char *tmp;
    tmp = cur;
    cur = next;
    next = tmp;
  }
  CudaCheckError();
  const double elapsed = hpc_gettime() - tstart;

  CudaSafeCall(cudaMemcpy(bmap.bmap, cur, bytes, cudaMemcpyDeviceToHost));

  fprintf(stderr, "Execution time: %f\n", elapsed);
  write_pgm(stdout, &bmap);
  free_pgm(&bmap);
  cudaFree(cur);
  cudaFree(next);
  return EXIT_SUCCESS;
}
