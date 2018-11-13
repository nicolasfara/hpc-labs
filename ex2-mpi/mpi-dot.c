/* */
/****************************************************************************
 *
 * mpi-dot.c - Parallel dot product using MPI.
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
 * --------------------------------------------------------------------------
 *
 * Compile with:
 * mpicc -std=c99 -Wall -Wpedantic mpi-dot.c -lm -o mpi-dot
 *
 * Run with:
 * mpirun -n 4 ./mpi-dot
 *
 ****************************************************************************/
#include <mpi.h> 
#include <stdio.h>
#include <stdlib.h>
#include <math.h> /* for fabs() */

/*
 * Compute sum { x[i] * y[i] }, i=0, ... n-1
 */
double dot( const double* x, const double* y, int n )
{
    double s = 0.0;
    int i;
    for (i=0; i<n; i++) {
        s += x[i] * y[i];
    }
    return s;
}

#ifndef USE_SCATTERV

/* This version uses MPI_Scatter() to distribute the input to the workers;
   the master process takes care of the leftovers */
double mpi_dot(double *x, double *y, int n)
{
    int my_rank, comm_sz;
    double *local_x, *local_y, result = 0.0, local_result = 0.0;
    int local_n;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    if (0 == my_rank) {
        printf("Computing dot product using MPI_Scatter()\n");
    }

    /* This version works for any value of n; the root takes care of
       the leftovers later on */
    local_n = n / comm_sz;

    /* All nodes (including the master) allocate the local vectors */
    local_x = (double*)malloc( local_n * sizeof(*local_x) );
    local_y = (double*)malloc( local_n * sizeof(*local_y) );

    /* Scatter vector x */
    MPI_Scatter( x,             /* sendbuf      */
                 local_n,       /* count; how many elements to send to _each_ destination */
                 MPI_DOUBLE,    /* sent datatype */
                 local_x,       /* recvbuf      */
                 local_n,       /* recvcount    */
                 MPI_DOUBLE,    /* received datatype */
                 0,             /* source       */
                 MPI_COMM_WORLD /* communicator */
                 );

    /* Scatter vector y*/
    MPI_Scatter( y,             /* sendbuf      */
                 local_n,       /* count; how many elements to send to _each_ destination */
                 MPI_DOUBLE,    /* sent datatype */
                 local_y,       /* recvbuf      */
                 local_n,       /* recvcount    */
                 MPI_DOUBLE,    /* received datatype */
                 0,             /* source       */
                 MPI_COMM_WORLD /* communicator */
                 );

    /* All nodes compute the local result */
    local_result = dot( local_x, local_y, local_n );

    /* Reduce (sum) the local dot products */
    MPI_Reduce( &local_result,  /* send buffer          */
                &result,        /* receive buffer       */
                1,              /* count                */
                MPI_DOUBLE,     /* datatype             */
                MPI_SUM,        /* operation            */
                0,              /* destination          */
                MPI_COMM_WORLD  /* communicator         */
                );

    if ( 0 == my_rank ) {
        /* the master handles the leftovers, if any */
        int i;
        for (i = local_n * comm_sz; i<n; i++) {
            result += x[i] * y[i];
        }
    }
    
    free(local_x);
    free(local_y);

    return result;
}

#else

/* This version uses MPI_Scatterv() to distribute the input to the
   workers */

double mpi_dot(double *x, double *y, int n)
{
    double *local_x, *local_y, result = 0.0, local_result = 0.0;
    int *sendcounts, *displs; /* used by MPI_Scatterv() */
    int local_n, i;
    int my_rank, comm_sz;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    if (0 == my_rank) {
        printf("Computing dot product using MPI_Scatterv()\n");
    }
    
    sendcounts = (int*)malloc(comm_sz * sizeof(*sendcounts));
    displs = (int*)malloc(comm_sz * sizeof(*displs));
    for (i=0; i<comm_sz; i++) {
        /* First, we compute the starting and ending position of each
           block (iend is actually one element _past_ the ending
           position) */
        const int istart = n*i/comm_sz;
        const int iend = n*(i+1)/comm_sz;
        /* Then, the length of block i is (iend - istart) */
        const int blklen = iend - istart;
        sendcounts[i] = blklen;
        displs[i] = istart;
    }

    local_n = sendcounts[my_rank]; /* how many elements I must handle */

    /* All nodes (including the master) allocate the local vectors */
    local_x = (double*)malloc( local_n * sizeof(*local_x) );
    local_y = (double*)malloc( local_n * sizeof(*local_y) );

    /* Scatter vector x */
    MPI_Scatterv( x,             /* sendbuf             */
                  sendcounts,    /* sendcounts          */
                  displs,        /* displacements       */
                  MPI_DOUBLE,    /* sent datatype       */
                  local_x,       /* recvbuf             */
                  sendcounts[my_rank], /* recvcount; actually, only local_n elements will be used */
                  MPI_DOUBLE,    /* received datatype   */
                  0,             /* source              */
                  MPI_COMM_WORLD /* communicator        */
                  );

    /* Scatter vector y*/
    MPI_Scatterv( y,             /* sendbuf             */
                  sendcounts,    /* sendcounts          */
                  displs,        /* displacements       */
                  MPI_DOUBLE,    /* sent datatype       */
                  local_y,       /* recvbuf             */
                  sendcounts[my_rank], /* recvcount; actually, only local_n elements will be used */
                  MPI_DOUBLE,    /* received datatype   */
                  0,             /* source              */
                  MPI_COMM_WORLD /* communicator        */
                  );

    /* All nodes compute the local result */
    local_result = dot( local_x, local_y, local_n );

    /* Reduce (sum) the local dot products */
    MPI_Reduce( &local_result,  /* send buffer          */
                &result,        /* receive buffer       */
                1,              /* count                */
                MPI_DOUBLE,     /* datatype             */
                MPI_SUM,        /* operation            */
                0,              /* destination          */
                MPI_COMM_WORLD  /* communicator         */
                );

    free(local_x);
    free(local_y);
    free(sendcounts);
    free(displs);

    return result;
}

#endif

int main( int argc, char* argv[] )
{
    double *x = NULL, *y = NULL, result = 0.0;
    int n = 1000;
    int my_rank, comm_sz;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    
    if ( argc > 1 ) {
        n = atoi(argv[1]);
    }

    /* [TODO] This is not a true parallel version, since the master
       does everything */    
    if ( 0 == my_rank ) {
        /* The master allocates the vectors */
        int i;
        x = (double*)malloc( n * sizeof(*x) );
        y = (double*)malloc( n * sizeof(*y) );
        for ( i=0; i<n; i++ ) {
            x[i] = i + 1.0;
            y[i] = 1.0 / x[i];
        }
        
        result = dot(x, y, n);
    }

    if (0 == my_rank) {
        printf("Dot product: %f\n", result);
        if ( fabs(result - n) < 1e-5 ) {
            printf("Check OK\n");
        } else {
            printf("Check failed: got %f, expected %f\n", result, (double)n);
        }
    }        
    
    free(x); /* if x == NULL, does nothing */
    free(y);

    MPI_Finalize();

    return 0;
}
