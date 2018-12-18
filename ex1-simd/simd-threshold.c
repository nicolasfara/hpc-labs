/* */
/****************************************************************************
 *
 * simd-threshold.c - Image thresholding
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
 * gcc -std=c99 -Wall -Wpedantic -O2 -march=native simd-threshold.c -o simd-threshold
 *
 * Run with:
 *
 * ./simd-threshold thr < input_file > output_file
 *
 * where 0 <= thr < 255
 *
 ****************************************************************************/

/* The following #define is required by posix_memalign() */
#define _XOPEN_SOURCE 600

#include "hpc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef unsigned char v16uc __attribute__((vector_size(16)));
#define VLEN (sizeof(v16uc)/sizeof(unsigned char))

typedef struct {
  int width;   /* Width of the image (in pixels) */
  int height;  /* Height of the image (in pixels) */
  int maxgrey; /* Don't care (used only by the PGM read/write routines) */
  unsigned char *bmap; /* buffer of width*height bytes; each element represents the gray level of a pixel (0-255) */
} img_t;

/* Read a PGM file from file |f|. This function is not very robust; it
   may fail on perfectly legal PGM images, but works for the provided
   cat.pgm file. */
void read_pgm( FILE *f, img_t* img )
{
  char buf[1024];
  const size_t BUFSIZE = sizeof(buf);
  char *s; 
  int nread, ret;

  /* Get the file type (must be "P5") */
  s = fgets(buf, BUFSIZE, f);
  if (0 != strcmp(s, "P5\n")) {
    fprintf(stderr, "Wrong file type %s\n", buf);
    exit(EXIT_FAILURE);
  }
  /* Get any comment and ignore it; does not work if there are
     leading spaces in the comment line */
  do {
    s = fgets(buf, BUFSIZE, f);
  } while (s[0] == '#');
  /* Get width, height */
  sscanf(s, "%d %d", &(img->width), &(img->height));
  /* get maxgrey; must be less than or equal to 255 */
  s = fgets(buf, BUFSIZE, f);
  sscanf(s, "%d", &(img->maxgrey));
  if ( img->maxgrey > 255 ) {
    fprintf(stderr, "Error: maxgray > 255 (%d)\n", img->maxgrey);
    exit(EXIT_FAILURE);
  }
  /* Get the binary data. Note that the pointer img->bmap must be
     properly aligned to allow SIMD instructions, because the
     compiler emits SIMD instructions for aligned load/stores only. */
  ret = posix_memalign((void**)&(img->bmap), __BIGGEST_ALIGNMENT__, (img->width)*(img->height));
  assert( 0 == ret );
  nread = fread(img->bmap, 1, (img->width)*(img->height), f);
  if ( (img->width)*(img->height) != nread ) {
    fprintf(stderr, "Error reading input file: expecting %d bytes, got %d\n", (img->width)*(img->height), nread);
    exit(EXIT_FAILURE);
  }
}

/**
 * Write a pgm image to file |f| 
 */
void write_pgm( FILE *f, const img_t* img )
{
  fprintf(f, "P5\n");
  fprintf(f, "# produced by cat-map\n");
  fprintf(f, "%d %d\n", img->width, img->height);
  fprintf(f, "%d\n", img->maxgrey);
  fwrite(img->bmap, 1, (img->width)*(img->height), f);
}

/**
 * Free img and set fields to invalid values.
 */
void free_pgm( img_t *img )
{
  img->width = img->height = img->maxgrey = -1;
  free(img->bmap);
  img->bmap = NULL;
}

/*
 * Set the pixels with value < thr to white (0), and all other ones to
 * black (255).
 */
void threshold( img_t* img, unsigned char thr )
{
  const int width = img->width;
  const int height = img->height;
  unsigned char *bmap = img->bmap;
  int i, j;

  const v16uc black = {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};
  const v16uc white = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  for (i=0; i<height; i++) {
    for (j=0; j<width - VLEN + 1; j += VLEN) {
      v16uc *pixel = (v16uc *) (bmap + i*width + j);
      const v16uc mask = (*pixel <= thr);
      *pixel = (mask & white) | (~mask & black);
      // Optimization: *pixel = (~mask & black);
      /*if (*pixel <= thr ) {
        *pixel = 0;
      } else {
        *pixel = 255;
      }*/
    }
  }
}

int main( int argc, char* argv[] )
{
  img_t bmap;
  int thr;
  double tstart, elapsed;

  if ( argc != 2 ) {
    fprintf(stderr, "Usage: %s thr < in.pgm > out.pgm\n", argv[0]);
    return EXIT_FAILURE;
  }
  thr = atoi(argv[1]);
  if (thr < 0 || thr >= 255) {
    fprintf(stderr, "FATAL: invalid threshold %d\n", thr);
    return EXIT_FAILURE;
  }
  read_pgm(stdin, &bmap);
  if ( bmap.width % VLEN ) {
    fprintf(stderr, "FATAL: the image width (%d) must be multiple of %d\n", bmap.width, (int)VLEN);
    return EXIT_FAILURE;
  }
  tstart = hpc_gettime();
  threshold(&bmap, thr);
  elapsed = hpc_gettime() - tstart;
  fprintf(stderr, "Executon time: %f\n", elapsed);
  write_pgm(stdout, &bmap);
  free_pgm(&bmap);
  return EXIT_SUCCESS;
}

// vim: set nofoldenable :
