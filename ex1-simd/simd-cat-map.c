/* */
/****************************************************************************
 *
 * simd-cat-map.c - Arnold's cat map using SIMD vector datatypes
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
 * gcc -std=c99 -Wall -Wpedantic -O2 -march=native -D_XOPEN_SOURCE=600 simd-cat-map.c -o simd-cat-map
 *
 * Run with:
 *
 * ./simd-cat-map k < input_file > output_file
 *
 * to compute the k-th iterate of the cat map.  Input and output files
 * are in PGM (Portable Graymap) format; see "man pgm" for details
 * (however, you do not need to know anything about the PGM formato;
 * functions are provided below to read and write a PGM file). The
 * file cat.pgm can be used as a test image.  Example:
 *
 * ./simd-cat-map 100 < cat.pgm > cat.100.pgm
 *
 * See https://en.wikipedia.org/wiki/Arnold%27s_cat_map for an explanation
 * of the cat map.
 *
 * To look at the assembly code produced by the compiler, use the command
 *
 * objdump -S simd-cat-map
 *
 * In this case, you might want to compile with:
 *
 * gcc -std=c99 -Wall -Wpedantic -g -ggdb -O0 -march=native -D_XOPEN_SOURCE=600 simd-cat-map.c -o simd-cat-map
 *
 * to include debug information and to prevent GCC from messing [too
 * much] with your code.
 *
 ****************************************************************************/

/* The following #define is required by posix_memalign() */
#define _XOPEN_SOURCE 600

#include "hpc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef int v4i __attribute__((vector_size(16)));
#define VLEN (sizeof(v4i)/sizeof(int))

typedef struct {
    int width;   /* Width of the image (in pixels) */
    int height;  /* Height of the image (in pixels) */
    int maxgrey; /* Don't care (used only by the PGM read/write routines) */
    unsigned char *bmap; /* buffer of width*height bytes; each byte represents the gray level of a pixel */
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
    /* Get the binary data */
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
 * Compute the |k|-th iterate of the cat map for image |img|. You must
 * implement this function, starting with a serial version, and then
 * adding OpenMP pragmas. 
 */
void cat_map( img_t* img, int k )
{
    const int N = img->width;
    unsigned char *cur = img->bmap;
    unsigned char *next;
    int x, y, i, ret;

    assert( img->width == img->height );

    ret = posix_memalign((void**)&next, __BIGGEST_ALIGNMENT__, N*N*sizeof(*next)); 
    assert( 0 == ret );

    for (y=0; y<N; y++) {
        /* [TODO] The SIMD version should compute the new position of
           four adjacent pixels (x,y), (x+1,y), (x+2,y), (x+3,y) using
           SIMD instructions. Assume that w (the image width) is
           always a multiple of VLEN. */
        for (x=0; x<N; x++) {
            int xold = x, xnew = xold;
            int yold = y, ynew = yold;
            for (i=0; i<k; i++) {
                xnew = (2*xold+yold) % N;
                ynew = (xold + yold) % N;
                xold = xnew;
                yold = ynew;
            }
            next[xnew + ynew*N] = cur[x+y*N];
        }
    }
    
    img->bmap = next;
    free(cur);
}

int main( int argc, char* argv[] )
{
    img_t bmap;
    int niter;
    double tstart, elapsed;
    
    if ( argc != 2 ) {
        fprintf(stderr, "Usage: %s niter < in.pgm > out.pgm\n\nExample: %s 684 < cat.pgm > out.pgm\n", argv[0], argv[0]);
        return EXIT_FAILURE;
    }
    niter = atoi(argv[1]);
    read_pgm(stdin, &bmap);
    if ( bmap.width != bmap.height ) {
        fprintf(stderr, "FATAL: width (%d) and height (%d) of the input image must be equal\n", bmap.width, bmap.height);
        return EXIT_FAILURE;
    }
    if ( bmap.width % VLEN ) {
        fprintf(stderr, "Error: this program expects the image width (%d) to be a multiple of %d\n", bmap.width, (int)VLEN);
        return EXIT_FAILURE;
    }
    tstart = hpc_gettime();
    cat_map(&bmap, niter);
    elapsed = hpc_gettime() - tstart;
    fprintf(stderr, "Executon time: %f\n", elapsed);
    write_pgm(stdout, &bmap);
    free_pgm(&bmap);
    return EXIT_SUCCESS;
}
