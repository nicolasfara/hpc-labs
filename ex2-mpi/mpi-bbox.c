/* */
/****************************************************************************
 *
 * mpi-bbox.c - Compute the bounding box of a set of rectangles
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
 * Compile with:
 * mpicc -std=c99 -Wall -Wpedantic mpi-bbox.c -lm -o mpi-bbox
 *
 * Run with:
 * mpirun -n 4 ./mpi-bbox bbox-1000.in
 *
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h> /* for fminf() */
#include <mpi.h>

/* Compute the bounding box of |n| rectangles whose opposite vertices
   have coordinates (|x1[i]|, |y1[i]|), (|x2[i]|, |y2[i]|). The
   opposite corners of the bounding box will be stored in (|xb1|,
   |yb1|), (|xb2|, |yb2|) */   
void bbox( const float *x1, const float *y1, const float* x2, const float *y2, int n,
           float *xb1, float *yb1, float *xb2, float *yb2 )
{
    int i;
    *xb1 = x1[0];
    *yb1 = y1[0];
    *xb2 = x2[0];
    *yb2 = y2[0];
    for (i=1; i<n; i++) {
        *xb1 = fminf( *xb1, x1[i] );
        *yb1 = fminf( *yb1, y1[i] );
        *xb2 = fmaxf( *xb2, x2[i] );
        *yb2 = fmaxf( *yb2, y2[i] );
    }
}

int main( int argc, char* argv[] )
{
    float *x1, *y1, *x2, *y2;
    float xb1, yb1, xb2, yb2;
    int N;
    int my_rank, comm_sz;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    if ( (0 == my_rank) && (argc != 2) ) {
        printf("Usage: %s [inputfile]\n", argv[0]);
        MPI_Abort(MPI_COMM_WORLD, -1);
    }

    x1 = y1 = x2 = y2 = NULL;

    /* [TODO] This is not a true parallel version since the master
       does everything */
    if ( 0 == my_rank ) {
        FILE *in = fopen(argv[1], "r");
        int i;
        if ( !in ) {
            fprintf(stderr, "Cannot open %s for reading\n", argv[1]);
            MPI_Abort(MPI_COMM_WORLD, -1);
        }
        fscanf(in, "%d", &N);
        x1 = (float*)malloc(N * sizeof(*x1));
        y1 = (float*)malloc(N * sizeof(*y1));
        x2 = (float*)malloc(N * sizeof(*x2));
        y2 = (float*)malloc(N * sizeof(*y2));
        for (i=0; i<N; i++) {
            fscanf(in, "%f %f %f %f", &x1[i], &y1[i], &x2[i], &y2[i]);
        }
        fclose(in);
        
        /* Compute the bounding box */
        bbox( x1, y1, x2, y2, N, &xb1, &yb1, &xb2, &yb2 );
        
        /* Print bounding box */
        printf("bbox: %f %f %f %f\n", xb1, yb1, xb2, yb2);
    }
    
    MPI_Finalize();

    return 0;
}
