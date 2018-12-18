/****************************************************************************
 *
 * cuda-knapsack.cu - Solve the 0/1 integer knapsack problem using CUDA
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
 * nvcc cuda-knapsack.cu -o cuda-knapsack
 *
 * Run with:
 * ./cuda-knapsack < knapsack-100-1000.in
 *
 ****************************************************************************/
#include "hpc.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>       /* for fmaxf() */
#include <assert.h>     /* for assert() */

/* Problem instance */
typedef struct {
  int C;          /* capacity             */
  int n;          /* number of items      */
  int *w;         /* array of n weights   */
  float *v;       /* array of n values    */
} knapsack_t;

/**
 * Given a set of n objects of weights w[0], ... w[n-1] and values
 * v[0], ... v[n-1], compute the maximum profit that can be obtained
 * by putting a subset of objects into a container of total capacity
 * C. Formally, the goal is to find a binary vector x[0], ... x[n-1]
 * such that:
 *
 * sum_{i=0}^{n-1} x[i] * v[i] is maximized
 *
 * subject to: sum_{i=0}^{n-1} x[i] * w[i] <= C
 *
 * This function uses the standard approach based on dynamic
 * programming; therefore, it requires space proportional to n*C
 */
float knapsack(int C, int n, int* w, float *v)
{
  const int NROWS = n;
  const int NCOLS = C+1;
  float *Pcur, *Pnext;
  float result;
  int i, j;

  /* [TODO] questi array andranno allocati nella memoria del device */    
  Pcur = (float*)malloc(NCOLS*sizeof(*Pcur)); assert(Pcur);
  Pnext = (float*)malloc(NCOLS*sizeof(*Pnext)); assert(Pnext);

  /* Inizializzazione: [TODO] volendo si puo' trasformare questo
     ciclo in un kernel CUDA, oppure si puo' far calcolare dalla CPU
     e successivamente trasferire Pcur nella memoria del device. */
  for (j=0; j<NCOLS; j++) {
    Pcur[j] = (j < w[0] ? 0.0 : v[0]);
  }
  /* Compute the DP matrix row-wise */
  for (i=1; i<NROWS; i++) {
    /* [TODO] Scrivere un kernel che esegua il ciclo seguente
       eseguendo "NCOLS" CUDA thread in parallelo */
    for (j=0; j<NCOLS; j++) {
      Pnext[j] = Pcur[j];
      if ( j>=w[i] ) {
        Pnext[j] = fmaxf(Pcur[j], Pcur[j - w[i]] + v[i]);
      }
    }
    /* Here, Pnext[j] is the maximum profit that can be obtained
       by putting a subset of items {0, 1, ... i} into a container
       of capacity j */
    float *tmp = Pcur;
    Pcur = Pnext;
    Pnext = tmp;
  }
  result = Pcur[NCOLS-1];
  free(Pcur); 
  free(Pnext);
  return result;
}

/* Read and allocate a problem instance from file |fin|; the file must
   conta, in order, C n w v. The problem instance can be deallocated
   with knapsack_free() */
void knapsack_load(FILE *fin, knapsack_t* k)
{
  int i;
  assert(k);
  fscanf(fin, "%d", &(k->C)); assert( k->C > 0 );
  fscanf(fin, "%d", &(k->n)); assert( k->n > 0 );
  k->w = (int*)malloc((k->n)*sizeof(int)); assert(k->w);
  k->v = (float*)malloc((k->n)*sizeof(float)); assert(k->v);
  for (i=0; i<(k->n); i++) {
    int nread = fscanf(fin, "%d %f", k->w + i, k->v + i);
    assert(2 == nread);
    assert(k->w[i] >= 0);
    assert(k->v[i] >= 0);
    /* fprintf(stderr, "%d %f\n", *(k->w + i), *(k->v + i)); */
  }
  fprintf(stderr, "Loaded instance with %d items, capacity %d\n", k->n, k->C);
}

/* Deallocate all memory used by a problem instance */
void knapsack_free(knapsack_t* k)
{
  assert(k);
  k->n = k->C = 0;
  free(k->w); k->w = NULL;
  free(k->v); k->v = NULL;
}

void knapsack_solve(const knapsack_t* k)
{
  assert(k);
  float result = knapsack(k->C, k->n, k->w, k->v);
  printf("Optimal profit: %f\n", result);
}

int main(int argc, char* argv[])
{
  knapsack_t k;

  if ( 1 != argc ) {
    fprintf(stderr, "Usage: %s < inputfile\n", argv[0]);
    return EXIT_FAILURE;
  }

  knapsack_load(stdin, &k);
  const double tstart = hpc_gettime();
  knapsack_solve(&k);
  const double elapsed = hpc_gettime() - tstart;
  /* Note that the execution time includes memory allocation and
     data movement to/from the GPU */
  fprintf(stderr, "Execution time: %f\n", elapsed);
  knapsack_free(&k);
  return EXIT_SUCCESS;
}
