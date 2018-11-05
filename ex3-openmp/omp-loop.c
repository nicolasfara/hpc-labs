/* */
/****************************************************************************
 *
 * omp-loop.c - Restructure loops to remove dependencies
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
 ****************************************************************************/
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Three small functions used below; you should not need to know what
   these functions do */
int f(int a, int b, int c) { return (a+b+c)/3; }
int g(int a, int b) { return (a+b)/2; }
int h(int a) { return (a > 10 ? 2*a : a-1); }

/****************************************************************************/

/**
 * Shift the elements of array |a| of length |n| one position to the
 * right; the rightmost element of |a| becomes the leftmost element of
 * the shifted array.
 */
void vec_shift_right_seq(int *a, int n)
{
  int i;
  int tmp = a[n-1];
  for (i=n-1; i>0; i--) {
    a[i] = a[i-1];
  }
  a[0] = tmp;
}

void vec_shift_right_par(int *a, int n)
{
  /* [TODO] This function should be a parallel version of
     vec_shift_right_seq(). Note that it is not possible to remove
     the loop-carried dependency by aligning loop
     iterations. However, one possible solution is to use a
     temporary array b[] and split the loop into two loops: the
     first loop copies all elements of a[] in the shifted position
     of b[] (i.e., a[i] goes to b[i+1]; the rightmost element of a[]
     goes into b[0]). Then the second loop simply copies b[] into
     a[]. There are other solutions that do not require a temporary
     array b[]. */
  const int num_threads = omp_get_max_threads();
  int rightmost[num_threads];

#pragma omp parallel
  {
    const int my_id = omp_get_thread_num();
    const int my_start = n * my_id / num_threads;
    const int my_end = n * (my_id + 1) / num_threads;
    rightmost[my_id] = a[my_end - 1];
    for (int i = my_end - 1; i > my_start; i--) {
      a[i] = a[i -1];
    }
#pragma omp barrier
    const int left_neighbor = (my_id > 0 ? my_id - 1 : num_threads - 1);
    a[my_start] = rightmost[left_neighbor];
  }
}
/****************************************************************************/

void test1_seq(int *a, int *b, int *c, int n)
{
  int i;
  a[0] = h(0);
  b[0] = c[0] = 0;
  b[1] = a[0] % 10;

  for (i=1; i<n-1; i++) {
    a[i] = h(i);
    b[i+1] = a[i] % 10;
    c[i] = a[i - 1];
  }
  a[n-1] = h(n-1);
  c[n-1] = a[n-2];
}

void test1_par(int *a, int *b, int *c, int n)
{
  /* [TODO] This function should be a parallel version of
     test1_seq(). Loop-carried dependencies can be removed by
     aligning loop iterations. Pay attention to array elements that
     might fall outside the bounds of the aligned loop. */
  int i;
  a[0] = h(0);
  b[0] = c[0] = 0;
  b[1] = a[0] % 10;
  c[1] = a[0];
#pragma omp parallel for
  for (i=1; i<n-2; i++) {
    a[i] = h(i);
    b[i+1] = a[i] % 10;
    c[i + 1] = a[i];
  }
  a[n - 2] = h(n - 2);
  b[n - 1] = a[n - 2] % 10;
  a[n-1] = h(n-1);
  c[n-1] = a[n-2];
}

/****************************************************************************/

/* This is a hack to convert 2D indexes to a linear index; this macro
   requires the existence of a variable "n" representing the number
   of columns of the matrix being indexed. A proper solution would
   be to use a C99 cast such as:

   int (*AA)[n] = (int (*)[n])A;

   then, AA can be indexes as AA[i][j]. Unfortunately, this triggers a
   bug in gcc 5.4.0+openMP (works with gcc 8.2.0+OpenMP)
   */
#define IDX(i,j) ((i)*n + (j))

/* A is a nxn matrix */
void test2_seq(int *A, int n)
{
  int i, j;
  int (*AA)[n] = (int (*)[n])A;
#pragma omp parallel for default(none) private(j) shared(AA, n)
  for (i=1; i<n; i++) {
    for (j=1; j<n-1; j++) {
      AA[i][j] = f(AA[i-1][j-1], AA[i-1][j], AA[i-1][j+1]);
    }
  }
}

void test2_par(int *A, int n)
{
  /* [TODO] This function should be a parallel vesion of
     test2_seq(). Suggestion: start by drawing the dependencies
     among the elements of matrix A[][] as they are computed.
     Then, observe that one of the loops (which one?) can be
     parallelized as is with a #pragma opm parallel for directive. */
}

/****************************************************************************/

void test3_seq(int *A, int n)
{
  int i, j;
  for (i=1; i<n; i++) {
    for (j=1; j<n; j++) {
      A[IDX(i,j)] = g(A[IDX(i,j-1)], A[IDX(i-1,j-1)]);
    }
  }
}

void test3_par(int *A, int n)
{
  /* [TODO] This function should be a parallel version of
     test3_seq(). Suggestion: start by drawing the dependencies
     among the elements of matrix A[][] as they are
     computed. Observe that it is not possible to put a "parallel
     for" directive on either loop.

     However, loops can be exchanged. If you do that... */
}

/****************************************************************************/

void test4_seq(int *A, int n)
{
  int i, j;
  for (i=1; i<n; i++) {
    for (j=1; j<n; j++) {
      A[IDX(i,j)] = f(A[IDX(i,j-1)], A[IDX(i-1,j-1)], A[IDX(i-1,j)]);
    }
  }
}

void test4_par(int *A, int n)
{
  /* [TODO] This function should be a parallel version of
     test3_seq(). Suggestion: this is basically the same example
     shown on slide 31 of "L05-parallelizing-loops", and can be
     solved using the "wavefront sweep" code shown on slide 34.

     There is a caveat: the code on slide 34 sweeps the _whole_
     matrix; in other words, variables i and j will assume all
     values starting from 0. The code of test4_seq() only process
     indexes where i>0 and j>0, so you need to add an "if" statement
     to skip the case where i==0 or j == 0. */
}

void fill(int *a, int n)
{
  a[0] = 31;
  for (int i=1; i<n; i++) {
    a[i] = (a[i-1] * 33 + 1) % 65535;
  }
}

int array_equal(int *a, int *b, int n)
{
  for (int i=0; i<n; i++) {
    if (a[i] != b[i]) { return 0; }
  }
  return 1;
}

int main( void )
{
  const int N = 1024;
  int *a1, *b1, *c1, *a2, *b2, *c2;

  /* Allocate enough space for all tests */
  a1 = (int*)malloc(N*N*sizeof(int));
  b1 = (int*)malloc(N*sizeof(int));
  c1 = (int*)malloc(N*sizeof(int));

  a2 = (int*)malloc(N*N*sizeof(int));
  b2 = (int*)malloc(N*sizeof(int));
  c2 = (int*)malloc(N*sizeof(int));

  printf("vec_shift_right_par()\t"); fflush(stdout);
  fill(a1, N); vec_shift_right_seq(a1, N);
  fill(a2, N); vec_shift_right_par(a2, N);
  if ( array_equal(a1, a2, N) ) {
    printf("OK\n");
  } else {
    printf("FAILED\n");
  }

  /* test1 */
  printf("test1_par()\t\t"); fflush(stdout);
  test1_seq(a1, b1, c1, N);
  test1_par(a2, b2, c2, N);
  if (array_equal(a1, a2, N) &&
      array_equal(b1, b2, N) &&
      array_equal(c1, c2, N)) {
    printf("OK\n");
  } else {
    printf("FAILED\n");
  }

  /* test2 */
  printf("test2_par()\t\t"); fflush(stdout);
  fill(a1, N*N); test2_seq(a1, N);
  fill(a2, N*N); test2_par(a2, N);
  if (array_equal(a1, a2, N*N)) {
    printf("OK\n");
  } else {
    printf("FAILED\n");
  }

  /* test3 */
  printf("test3_par()\t\t"); fflush(stdout);
  fill(a1, N*N); test3_seq(a1, N);
  fill(a2, N*N); test3_par(a2, N);
  if (array_equal(a1, a2, N*N)) {
    printf("OK\n");
  } else {
    printf("FAILED\n");
  }

  /* test4 */
  printf("test4_par()\t\t"); fflush(stdout);
  fill(a1, N*N); test4_seq(a1, N);
  fill(a2, N*N); test4_par(a2, N);
  if (array_equal(a1, a2, N*N)) {
    printf("OK\n");
  } else {
    printf("FAILED\n");
  }

  free(a1); free(b1); free(c1);
  free(a2); free(b2); free(c2);

  return EXIT_SUCCESS;
}

// vim: set nofoldenable :
