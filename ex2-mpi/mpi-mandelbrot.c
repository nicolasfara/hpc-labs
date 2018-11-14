/* */
/****************************************************************************
 *
 * mpi-mandelbrot.c - Computation of the Mandelbrot set with MPI
 *
 * Written in 2017 by Moreno Marzolla <moreno.marzolla(at)unibo.it>
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
 * --------------------------------------------------------------------------
 *
 * Compile with
 * mpicc -std=c99 -Wall -Wpedantic mpi-mandelbrot.c -o mpi-mandelbrot
 *
 * run with:
 * mpirun -n 4 ./mpi-mandelbrot
 *
 * The master creates a file "mandelbrot.ppm" with the final image.
 *
 ****************************************************************************/
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

const int MAXIT = 100;

typedef struct {
  uint8_t r;  /* red   */
  uint8_t g;  /* green */
  uint8_t b;  /* blue  */
} pixel_t;

/* color gradient from https://stackoverflow.com/questions/16500656/which-color-gradient-is-used-to-color-mandelbrot-in-wikipedia */
const int colors[][3] = {
  {66, 30, 15}, /* r, g, b */
  {25, 7, 26},
  {9, 1, 47},
  {4, 4, 73},
  {0, 7, 100},
  {12, 44, 138},
  {24, 82, 177},
  {57, 125, 209},
  {134, 181, 229},
  {211, 236, 248},
  {241, 233, 191},
  {248, 201, 95},
  {255, 170, 0},
  {204, 128, 0},
  {153, 87, 0},
  {106, 52, 3} };
const int NCOLORS = sizeof(colors)/sizeof(colors[0]);

/*
 * Iterate the recurrence:
 *
 * z_0 = 0;
 * z_{n+1} = z_n^2 + cx + i*cy;
 *
 * Returns the first n such that z_n > |bound|, or |MAXIT| if z_n is below
 * |bound| after |MAXIT| iterations.
 */
int iterate( float cx, float cy )
{
  float x = 0.0f, y = 0.0f, xnew, ynew;
  int it;
  for ( it = 0; (it < MAXIT) && (x*x + y*y <= 2.0*2.0); it++ ) {
    xnew = x*x - y*y + cx;
    ynew = 2.0*x*y + cy;
    x = xnew;
    y = ynew;
  }
  return it;
}

/* Draw the rows of the Mandelbrot set from |ystart| (inclusive) to
   |yend| (excluded) to the bitmap pointed to by |p|. Note that |p|
   must point to the beginning of the bitmap where the portion of
   image will be stored; in other words, this function writes to
   pixels p[0], p[1], ... */
void draw_lines( int ystart, int yend, pixel_t* p, int xsize, int ysize )
{
  int x, y;
  for ( y = ystart; y < yend; y++) {
    for ( x = 0; x < xsize; x++ ) {
      const float cx = -2.5 + 3.5 * (float)x / (xsize - 1);
      const float cy = 1 - 2.0 * (float)y / (ysize - 1);
      const int v = iterate(cx, cy);
      if (v < MAXIT) {
        p->r = colors[v % NCOLORS][0];
        p->g = colors[v % NCOLORS][1];
        p->b = colors[v % NCOLORS][2];
      } else {
        p->r = p->g = p->b = 0;
      }
      p++;
    }
  }
}

int main( int argc, char *argv[] )
{
  int my_rank, comm_sz;
  FILE *out = NULL;
  const char* fname="mandelbrotMPI.ppm";
  pixel_t *bitmap = NULL;
  int xsize, ysize;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

  if ( argc > 1 ) {
    ysize = atoi(argv[1]);
  } else {
    ysize = 1024;
  }

  xsize = ysize * 1.4;

  if ( 0 == my_rank ) {
    out = fopen(fname, "w");
    if ( !out ) {
      fprintf(stderr, "Error: cannot create %s\n", fname);
      MPI_Abort(MPI_COMM_WORLD, -1);
    }

    /* Write the header of the output file */
    fprintf(out, "P6\n");
    fprintf(out, "%d %d\n", xsize, ysize);
    fprintf(out, "255\n");

    /* Allocate the complete bitmap */
    bitmap = (pixel_t*)malloc(xsize*ysize*sizeof(*bitmap));
  }

  const int start = ysize * my_rank / comm_sz;
  const int end = ysize * (my_rank + 1) / comm_sz;
  const int size = end - start;
  const int send_size = size * xsize * sizeof(pixel_t);
  pixel_t *local_bitmap = (pixel_t *) malloc(xsize * size * sizeof(pixel_t));

  int *displs = (int*) malloc(sizeof(int) * comm_sz);
  int *recvcounts = (int*) malloc(sizeof(int) * comm_sz);

  for (int i = 0; i < comm_sz; i++) {
    const int my_start = ysize * i / comm_sz;
    const int my_end = ysize * (i + 1) / comm_sz;
    const int my_size = my_end - my_start;
    const int my_send_size = my_size * xsize * sizeof(pixel_t);
    displs[i] = my_start * xsize * sizeof(pixel_t);
    recvcounts[i] = my_send_size;
  }

  draw_lines(start, end, local_bitmap, xsize, ysize);

  MPI_Gatherv(local_bitmap, send_size, MPI_BYTE, bitmap, recvcounts, displs, MPI_BYTE, 0, MPI_COMM_WORLD);

  if(0 == my_rank) {
    fwrite(bitmap, sizeof(*bitmap), xsize*ysize, out);
    fclose(out);
    free(bitmap);
  }

  free(local_bitmap);
  free(recvcounts);
  free(displs);

  MPI_Finalize();

  return 0;
}

// vim: set nofoldenable :
