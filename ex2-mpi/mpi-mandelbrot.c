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

const int MAXIT = 1000;

typedef struct {
  uint8_t r;  /* red   */
  uint8_t g;  /* green */
  uint8_t b;  /* blue  */
} pixel_t;

/* color gradient from https://stackoverflow.com/questions/16500656/which-color-gradient-is-used-to-color-mandelbrot-in-wikipedia */
//const int colors[][3] = {
//  {66, 30, 15}, /* r, g, b */
//  {25, 7, 26},
//  {9, 1, 47},
//  {4, 4, 73},
//  {0, 7, 100},
//  {12, 44, 138},
//  {24, 82, 177},
//  {57, 125, 209},
//  {134, 181, 229},
//  {211, 236, 248},
//  {241, 233, 191},
//  {248, 201, 95},
//  {255, 170, 0},
//  {204, 128, 0},
//  {153, 87, 0},
//  {106, 52, 3} };

// from 2241CE - CA370C
//const int colors[][3] = {
//  {34, 65, 206}, /* r, g, b */
//  {36, 64, 202},
//  {39, 64, 199},
//  {42, 64, 196},
//  {44, 64, 193},
//  {47, 64, 190},
//  {52, 63, 184},
//  {55, 63, 181},
//  {58, 63, 178},
//  {60, 63, 175},
//  {63, 63, 172},
//  {66, 63, 169},
//  {68, 62, 165},
//  {71, 62, 162},
//  {74, 62, 159},
//  {76, 62, 156}, //--
//  {79, 62, 153},
//  {82, 62, 150},
//  {84, 61, 147},
//  {87, 61, 144},
//  {90, 61, 141},
//  {92, 61, 138},
//  {95, 61, 135},
//  {98, 61, 132},
//  {100, 61, 129},
//  {103, 60, 125},
//  {106, 60, 122},
//  {108, 60, 119},
//  {111, 60, 116},
//  {114, 60, 113},
//  {116, 60, 110},
//  {119, 59, 107}, //--
//  {79, 62, 153},
//  {82, 62, 150},
//  {84, 61, 147},
//  {87, 61, 144},
//  {90, 61, 141},
//  {92, 61, 138},
//  {95, 61, 135},
//  {98, 61, 132},
//  {100, 61, 129},
//  {103, 60, 125},
//  {106, 60, 122},
//  {108, 60, 119},
//  {111, 60, 116},
//  {114, 60, 113},
//  {116, 60, 110} };

const int colors[][3] = {
{34, 206, 90},
{35, 205, 89},
{37, 205, 88},
{39, 204, 87},
{40, 204, 86},
{42, 204, 86},
{44, 203, 85},
{45, 203, 84},
{47, 203, 83},
{49, 202, 82},
{50, 202, 82},
{52, 202, 81},
{54, 201, 80},
{56, 201, 79},
{57, 201, 78},
{59, 200, 78},
{61, 200, 77},
{62, 200, 76},
{64, 199, 75},
{66, 199, 75},
{67, 199, 74},
{69, 198, 73},
{71, 198, 72},
{73, 198, 71},
{74, 197, 71},
{76, 197, 70},
{78, 197, 69},
{79, 196, 68},
{81, 196, 67},
{83, 196, 67},
{84, 195, 66},
{86, 195, 65},
{88, 195, 64},
{90, 194, 63},
{91, 194, 63},
{93, 193, 62},
{95, 193, 61},
{96, 193, 60},
{98, 192, 60},
{100, 192, 59},
{101, 192, 58},
{103, 191, 57},
{105, 191, 56},
{106, 191, 56},
{108, 190, 55},
{110, 190, 54},
{112, 190, 53},
{113, 189, 52},
{115, 189, 52},
{117, 189, 51},
{118, 188, 50},
{120, 188, 49},
{122, 188, 49},
{123, 187, 48},
{125, 187, 47},
{127, 187, 46},
{129, 186, 45},
{130, 186, 45},
{132, 186, 44},
{134, 185, 43},
{135, 185, 42},
{137, 185, 41},
{139, 184, 41},
{140, 184, 40},
{142, 184, 39},
{144, 183, 38},
{146, 183, 38},
{147, 182, 37},
{149, 182, 36},
{151, 182, 35},
{152, 181, 34},
{154, 181, 34},
{156, 181, 33},
{157, 180, 32},
{159, 180, 31},
{161, 180, 30},
{162, 179, 30},
{164, 179, 29},
{166, 179, 28},
{168, 178, 27},
{169, 178, 26},
{171, 178, 26},
{173, 177, 25},
{174, 177, 24},
{176, 177, 23},
{178, 176, 23},
{179, 176, 22},
{181, 176, 21},
{183, 175, 20},
{185, 175, 19},
{186, 175, 19},
{188, 174, 18},
{190, 174, 17},
{191, 174, 16},
{193, 173, 15},
{195, 173, 15},
{196, 173, 14},
{198, 172, 13},
{200, 172, 12},
{202, 172, 12} };

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

  xsize = ysize * 1.777777778;

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
