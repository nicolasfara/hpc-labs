/* */
/****************************************************************************
 *
 * omp-merge-sort.c - Merge Sort with OpenMP tasks
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
 * ---------------------------------------------------------------------------
 *
 * This program generates a random permutation of the first n integers
 * 0, 1, ... n-1 and sorts it using Merge Sort. This implementation
 * uses selection sort to sort small subvectors, in order to limit the
 * number of recursive calls.
 *
 * The goal of this exercise is to parallelize the program using
 * OpenMP tasks.
 *
 * Compile with:
 *
 * gcc -fopenmp -std=c99 -Wall -Wpedantic omp-mergesort.c -o omp-mergesort
 *
 * Run with:
 *
 * ./omp-mergesort 50000
 *
 ****************************************************************************/
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

int min(int a, int b)
{
  return (a < b ? a : b);
}

void swap(int* a, int* b)
{
  int tmp = *a;
  *a = *b;
  *b = tmp;
}

/**
 * Sort v[low..high] using selection sort. This function will be used
 * for small vectors only. Do not parallelize this.
 */
void selectionsort(int* v, int low, int high)
{
  int i, j;
  for (i=low; i<high; i++) {
    for (j=i+1; j<=high; j++) {
      if (v[i] > v[j]) {
        swap(&v[i], &v[j]);
      }
    }
  }
}

/**
 * Merge src[low..mid] with src[mid+1..high], put the result in
 * dst[low..high].
 *
 * Do not parallelize this function (in principle it could be done,
 * but it is very difficult:
 * http://www.drdobbs.com/parallel/parallel-merge/229204454
 * https://en.wikipedia.org/wiki/Merge_algorithm#Parallel_merge )
 */
void merge(int* src, int low, int mid, int high, int* dst)
{
  int i=low, j=mid+1, k=low;
  while (i<=mid && j<=high) {
    if (src[i] <= src[j]) {
      dst[k] = src[i++];
    } else {
      dst[k] = src[j++];
    }
    k++;
  }
  /* Handle remaining elements */
  while (i<=mid) {
    dst[k] = src[i++];
    k++;
  }
  while (j<=high) {
    dst[k] = src[j++];
    k++;
  }
}

/**
 * Sort v[i..j] using the recursive version of MergeSort; the array
 * tmp[i..j] is used as a temporary buffer (the caller is responsible
 * for providing a suitably sized array tmp).
 */
void mergesort_rec(int* v, int i, int j, int* tmp)
{
  const int cutoff = 16;
  /* If the portion to be sorted is smaller than the cutoff, use
     selectoin sort. This is a widely used optimization that limits
     the overhead of recursion for small vectors. */
  if ( j - i + 1 < cutoff ) 
    selectionsort(v, i, j);
  else {
    const int m = (i+j)/2;
    /* [TODO] The two recursive invocation of mergesort_rec() do
       not interfere each other; therefore, they can run in
       parallel. Create two OpenMP tasks to sort the first and
       second half of the array; then, you wait for all tasks to
       complete before merging the results. */
#pragma omp task
    mergesort_rec(v, i, m, tmp);
#pragma omp task
    mergesort_rec(v, m+1, j, tmp);
    /* When using OpenMP, we must wait here for the recursive
       invocations of mergesort_rec() to terminate before merging
       the result */
#pragma omp taskwait
    merge(v, i, m, j, tmp);
    /* copy the sorted data back to v */
    memcpy(v+i, tmp+i, (j-i+1)*sizeof(v[0]));
  }
}

/**
 * Sort v[] of length n using Merge Sort; after allocating a temporary
 * array with the same size of a (used for merging), this function
 * just calls mergesort_rec with the appropriate parameters.  After
 * mergesort_rec terminates, the temporary array is deallocated.
 */
void mergesort(int *v, int n)
{
  int* tmp = (int*)malloc(n*sizeof(v[0]));
  /* [TODO] Parallelize the body of this function. You should create
     a pool of thread here, and ensure that only one thread calls
     mergesort_rec() to start the recursion. */ 
  mergesort_rec(v, 0, n-1, tmp);
  free(tmp);
}

/* Returns a random integer in the range [a..b], inclusive */
int randab(int a, int b)
{
  return a + rand() % (b-a+1);
}

/**
 * Fills a[] with a random permutation of the intergers 0..n-1; the
 * caller is responsible for allocating a
 */
void fill(int* a, int n)
{
  int i;
  for (i=0; i<n; i++) {
    a[i] = (int)i;
  }
  for (i=0; i<n-1; i++) {
    int j = randab(i, n-1);
    swap(a+i, a+j);
  }
}

/* Return 1 iff a[] contains the values 0, 1, ... n-1, in that order */
int check(int* a, int n)
{
  int i;
  for (i=0; i<n; i++) {
    if ( a[i] != i ) {
      fprintf(stderr, "Expected a[%d]=%d, got %d\n", i, i, a[i]);
      return 0;
    }
  }
  return 1;
}

int main( int argc, char* argv[] )
{
  int n = 100000;
  int *a;

  if ( argc > 2 ) {
    fprintf(stderr, "Usage: %s [n]\n", argv[0]);
    return EXIT_FAILURE;
  }

  if ( argc > 1 ) {
    n = atoi(argv[1]);
  }

  a = (int*)malloc(n*sizeof(a[0])); assert(a);

  fill(a, n);
  printf("Sorting %d elements...", n); fflush(stdout);
  const double tstart = omp_get_wtime();
#pragma omp parallel
#pragma omp master
  mergesort(a, n);
  const double elapsed = omp_get_wtime() - tstart;
  printf("done\n");
  const int ok = check(a, n);
  printf("Check %s\n", (ok ? "OK" : "failed"));
  printf("Elapsed time: %f\n", elapsed);

  free(a);

  return EXIT_SUCCESS;
}

// vim: set nofoldenable :
