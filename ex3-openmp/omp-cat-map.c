/* */
/****************************************************************************
 *
 * omp-cat-map.c - Arnold's cat map
 *
 * Written in 2016 by Moreno Marzolla <moreno.marzolla(at)unibo.it>
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
 *
 * gcc -fopenmp -std=c99 -Wall -Wpedantic omp-cat-map.c -o omp-cat-map
 *
 * Run with:
 *
 * ./omp-cat-map [k] < input_file > output_file
 *
 * to compute the k-th iterate of the cat map.  Input and output files
 * are in PGM (Portable Graymap) format. The file cat.pgm can be used
 * as a test image. Example:
 *
 * ./omp-cat-map 100 < cat.pgm > cat-100.pgm
 *
 * See https://en.wikipedia.org/wiki/Arnold%27s_cat_map for an explanation
 * of the cat map.
 *
 ****************************************************************************/
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct {
  int width;   /* Width of the image (in pixels) */
  int height;  /* Height of the image (in pixels) */
  int maxgrey; /* Don't care (used only by the PGM read/write routines) */
  unsigned char *bmap; /* buffer of width*height bytes; each byte represents a pixel */
} img_t;

/**
 * Read a PGM image |img| from file |f|. This function is not very
 * robust; it may fail on perfectly legal PGM images, but works for
 * the image(s) provided.
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
  /* Get any comment and ignore it. Note: does not work if there are
     leading spaces in the comment line */
  do {
    s = fgets(buf, BUFSIZE, f);
  } while (s[0] == '#');
  /* Get width, height */
  sscanf(s, "%d %d", &(img->width), &(img->height));
  /* get maxgrey; must be <= 255 */
  s = fgets(buf, BUFSIZE, f);
  sscanf(s, "%d", &(img->maxgrey));
  if ( img->maxgrey > 255 ) {
    fprintf(stderr, "FATAL: maxgray > 255 (%d)\n", img->maxgrey);
    exit(EXIT_FAILURE);
  }
  /* Get the binary data */
  img->bmap = (unsigned char*)malloc((img->width)*(img->height)*sizeof(unsigned char));
  nread = fread(img->bmap, 1, (img->width)*(img->height), f);
  if ( (img->width)*(img->height) != nread ) {
    fprintf(stderr, "FATAL: error reading input file: expecting %d bytes, got %d\n", (img->width)*(img->height), nread);
    exit(EXIT_FAILURE);
  }
}

/**
 * Write image |img| to file |f|
 */
void write_pgm( FILE *f, const img_t* img )
{
  fprintf(f, "P5\n");
  fprintf(f, "# produced by omp-cat-map\n");
  fprintf(f, "%d %d\n", img->width, img->height);
  fprintf(f, "%d\n", img->maxgrey);
  fwrite(img->bmap, 1, (img->width)*(img->height), f);
}

/**
 * Deallocate |img| and set all other fields to dummy values.
 */
void free_pgm( img_t * img )
{
  free(img->bmap);
  img->bmap = NULL;
  img->width = img->height = img->maxgrey = -1;
}

/**
 * Compute the |k|-th iterate of the cat map for image |img|. The
 * width and height of the image must be equal. This function must
 * replace the bitmap of |img| with the one resulting after ierating
 * |k| times the cat map. To do so, the function allocates a temporary
 * bitmap with the same size of the original one, so that it reads one
 * pixel from the "old" image and copies it to the "new" image. After
 * each iteration of the cat map, the role of the two bitmaps are
 * exchanged.
 */
void cat_map( img_t* img, int k )
{
  int i, x, y;
  const int N = img->width;
  unsigned char *cur = img->bmap;
  unsigned char *next = (unsigned char*)malloc( N*N*sizeof(unsigned char) );
  unsigned char *tmp;

  assert( img->width == img->height );

  for (i=0; i<k; i++) {
#pragma omp parallel for schedule(static, 8) default(none) private(x) shared(cur, next)
    for (y=0; y<N; y++) {
      for (x=0; x<N; x++) {
        const int xnext = (2*x+y) % N;
        const int ynext = (x + y) % N;
        next[xnext + ynext*N] = cur[x+y*N];
      }
    }
    /* Swap old and new */
    tmp = cur;
    cur = next;
    next = tmp;
  }
  img->bmap = cur;
  free(next);
}


int main( int argc, char* argv[] )
{
  img_t img;
  int niter;
  double tstart, tend;

  if ( argc != 2 ) {
    fprintf(stderr, "Usage: %s niter\n", argv[0]);
    return EXIT_FAILURE;
  }
  niter = atoi(argv[1]);
  read_pgm(stdin, &img);

  if ( img.width != img.height ) {
    fprintf(stderr, "FATAL: width (%d) and height (%d) of the input image must be equal\n", img.width, img.height);
    return EXIT_FAILURE;
  }

  tstart = omp_get_wtime();
  cat_map(&img, niter);
  tend = omp_get_wtime();
  fprintf(stderr, "\nExecution time (normal)\n\t%d iterations in %f sec = %f it/sec\n", niter, tend - tstart, niter / (tend - tstart));
  write_pgm(stdout, &img);

  free_pgm( &img );
  return EXIT_SUCCESS;
}

// vim: set nofoldenable :
