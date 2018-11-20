/* */
/****************************************************************************
 *
 * mpi-rule30.c - Rule30 Cellular Automaton with MPI
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
 * This program implements the "rule 30 CA" as described in
 * https://en.wikipedia.org/wiki/Rule_30 
 *
 * Compile with:
 * mpicc -std=c99 -Wall -Wpedantic mpi-rule30.c -o mpi-rule30
 *
 * Run with:
 * mpirun -n 4 ./mpi-rule30 1024 1024
 *
 ****************************************************************************/
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

/* Note: the MPI datatype corresponding to "signed char" is MPI_CHAR */
typedef signed char cell_t;

/* number of ghost cells on each side; currently, this program assumes
   HALO == 1. */
const int HALO = 1;

/**
 * Given the current state of the CA, compute the next state. |ext_n|
 * is the number of cells PLUS the ghost cells. This function assumes
 * that the first and last cell of |cur| are ghost cells, and
 * therefore their values are used to compute |next| but are not
 * updated on the |next| array.
 */
void step( const cell_t *cur, cell_t *next, int ext_n )
{
    int i;
    for (i=HALO; i<ext_n-HALO; i++) {
        const cell_t east = cur[i-1];
        const cell_t center = cur[i];
        const cell_t west = cur[i+1];
        next[i] = ( (east && !center && !west) ||
                    (!east && !center && west) ||
                    (!east && center && !west) ||
                    (!east && center && west) );
    }
}

/**
 * Initialize the domain; all cells are 0, with the exception of a
 * single cell in the middle of the domain. |ext_n| is the width of the
 * domain PLUS the ghost cells.
 */
void init_domain( cell_t *cur, int ext_n )
{
    int i;
    for (i=0; i<ext_n; i++) {
        cur[i] = 0;
    }
    cur[ext_n/2] = 1;
}

/**
 * Dump the current state of the automaton to PBM file |out|. |ext_n|
 * is the true width of the domain PLUS the ghost cells.
 */
void dump_state( FILE *out, const cell_t *cur, int ext_n )
{
    int i;
    for (i=HALO; i<ext_n-HALO; i++) {
        fprintf(out, "%d ", cur[i]);
    }
    fprintf(out, "\n");
}

int main( int argc, char* argv[] )
{
    const char *outname = "rule30.pbm";
    FILE *out = NULL;
    int width, ext_width, steps = 1024, s;        
    /* |cur| is the memory buffer containint |width| elements; this is
       the full state of the CA. */
    cell_t *cur = NULL, *next = NULL, *tmp;
    int my_rank, comm_sz;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    if ( 0 == my_rank && argc > 3 ) {
        fprintf(stderr, "Usage: %s [width [steps]]\n", argv[0]);
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }
    
    if ( argc > 1 ) {
        width = atoi(argv[1]);
    } else {
        width = comm_sz * 256;
    }

    if ( argc > 2 ) {
        steps = atoi(argv[2]);
    }

    if ( (0 == my_rank) && (width % comm_sz) ) {
        printf("The image width (%d) must be a multiple of comm_sz (%d)\n", width, comm_sz);
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    /* ext_width is the width PLUS the halo */    
    ext_width = width + 2*HALO;
    
    /* The master creates the output file */    
    if ( 0 == my_rank ) {
        out = fopen(outname, "w");
        if ( !out ) {
            fprintf(stderr, "Cannot create %s\n", outname);
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
        fprintf(out, "P1\n");
        fprintf(out, "# produced by %s %d %d\n", argv[0], width, steps);
        fprintf(out, "%d %d\n", width, steps);

        /* Initialize the domain */
        cur = (cell_t*)malloc( ext_width * sizeof(*cur) );
        next = (cell_t*)malloc( ext_width * sizeof(*next) );
        init_domain(cur, ext_width);
    }

    /* [TODO] The following code already contains some pieces that
       need to be filled in for the parallel version. */
    
    /* [TODO] compute the rank of the next and previous process on the
       chain. These will be used to exchange the boundary */
    
    /*
    const int rank_next = ...
    const int rank_prev = ...
    */
    
    /* [TODO] size of each local domain; this will be set to |width /
       comm_sz + 2*HALO|, since it must include the ghost cells */
    /*
    const int local_width = ...
    */
    
    /* [TODO] |local_cur| and |local_next| are the local domains,
       handled by each MPI process. They both have |local_width|
       elements each */
    /*
    cell_t *local_cur = ...
    cell_t *local_next = ...
    */
    
    for (s=0; s<steps; s++) {

        /* [TODO] Here you should scatter the |cur| array into
           |local_cur|; we are assuming that the width of the domain
           is multiple of comm_sz. */
        /*
        MPI_Scatter( sendbuf,
                     sendcount,
                     datatype,
                     recvbuf,
                     recvcount,
                     datatype,
                     root,
                     MPI_COMM_WORLD
                     );        
        */

        /* This is OK; do not modify it */
        if ( 0 == my_rank ) {
            /* Dump the current state to the output image */
            dump_state(out, cur, ext_width);
        }

        /* [TODO] the following "if" should be modified to allow
           neighbors to exchange ghost cells; two MPI_Sendrecv
           operations are required to do so. */
        /* Receive left boundary from left neighbor; send
           right boundary to right neighbor;
        MPI_Sendrecv( sendbuf,
                      sendcount,
                      datatype,
                      dest,
                      sendtag,
                      recvbuf,
                      recvcount,
                      datatype,
                      source,
                      recvtag.
                      MPI_COMM_WORLD,
                      MPI_STATUS_IGNORE
                      );
        */

        /* Receive right boundary from right neighbor; send
           left boundary to left neighbor;
        MPI_Sendrecv( sendbuf,
                      sendcount,
                      datatype,
                      dest,
                      sendtag,
                      recvbuf,
                      recvcount,
                      datatype,
                      source,
                      recvtag.
                      MPI_COMM_WORLD,
                      MPI_STATUS_IGNORE
                      );
        */    
        if (0 == my_rank) {
            cur[0] = cur[ext_width-2*HALO];
            cur[ext_width-HALO] = cur[HALO];
        }

        /* [TODO] replace the following block; each process calls
           |step()| on its local domain */
        if (0 == my_rank) {
            /* Compute the next state */
            step(cur, next, ext_width);
        }

        /* [TODO] here we gather the local domains into the
           |cur| array; indeed, in the parallel version the
           master does not need |next| at all */
        /*
        MPI_Gather( sendbuf,
                    sendcount,
                    datatype,
                    recvbuf,
                    recvcount,
                    datatype,
                    root,
                    MPI_COMM_WORLD
                    );
        */

        /* [TODO] Replace the following block; each process
           exchanges local_cur and local_next */
        if (0 == my_rank) {
            /* swap cur and next */
            tmp = cur;
            cur = next;
            next = tmp;
        }
    }
    free(next);

    /* 
    free(local_cur);
    free(local_next);
    */
    
    if ( 0 == my_rank ) {
        fclose(out);
        free(cur);
    }
    
    MPI_Finalize();

    return EXIT_SUCCESS;
}
