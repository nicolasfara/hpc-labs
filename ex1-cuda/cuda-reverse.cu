/* */
/****************************************************************************
 *
 * cuda-reverse.cu - Array reversal with CUDA
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
 * ---------------------------------------------------------------------------
 *
 * Compile with:
 * nvcc cuda-reverse.cu -o cuda-reverse
 *
 * Run with:
 * ./cuda-reverse [n]
 *
 * Example:
 * ./cuda-reverse
 *
 ****************************************************************************/
#include "hpc.h"
#include <stdio.h>
#include <math.h>
#include <assert.h>

/* Reverse in[] into out[].
   [TODO] This function should be rewritten as a kernel. */
void reverse( int *in, int *out, int n )
{
    int i;
    for (i=0; i<n; i++) {
        out[n - 1 - i] = in[i];
    }
}

/* In-place reversal of in[] into itself.
   [TODO] This function should be rewritten as a kernel. */
void inplace_reverse( int *in, int n )
{
    int i = 0, j = n-1;
    while (i < j) {
        int tmp = in[j];
        in[j] = in[i];
        in[i] = tmp;
        j--;
        i++;
    }
}

void fill( int *x, int n )
{
    int i;
    for (i=0; i<n; i++) {
        x[i] = i;
    }
}

int check( int *x, int n )
{
    int i;
    for (i=0; i<n; i++) {
        if (x[i] != n - 1 - i) {
            fprintf(stderr, "Test FAILED: x[%d]=%d, expected %d\n", i, x[i], n-1-i);
            return 0;
        }
    }
    printf("Test OK\n");
    return 1;
}

int main( int argc, char* argv[] )
{
    int *h_in, *h_out;  /* host copy of array in[] and out[] */
    int n = 1024*1024;
    const int max_len = 512*1024*1024;
    
    if ( argc > 2 ) {
        fprintf(stderr, "Usage: %s [n]\n", argv[0]);
        return EXIT_FAILURE;
    }

    if ( argc > 1 ) {
        n = atoi(argv[1]);
    }

    if ( n > max_len ) {
        fprintf(stderr, "FATAL: the maximum length is %d\n", max_len);
        return EXIT_FAILURE;
    }
    
    const size_t size = n * sizeof(*h_in);

    /* Allocate host copy of in[] and out[] */
    h_in = (int*)malloc(size); assert(h_in);
    fill(h_in, n);
    h_out = (int*)malloc(size); assert(h_out);

    /* Reverse
       [TODO] Rewrite as a kernel launch. */
    printf("Reverse %d elements... ", n);
    reverse(h_in, h_out, n);    
    check(h_out, n);

    /* In-place reverse 
       [TODO] Rewrite as a kernel launch. */
    printf("In-place reverse %d elements... ", n);
    inplace_reverse(h_in, n);    
    check(h_in, n);

    /* Cleanup */
    free(h_in); free(h_out);
    return EXIT_SUCCESS;
}
