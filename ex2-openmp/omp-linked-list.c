/* */
/******************************************************************************
 *
 * omp-linked-list.c - Linked list processing with OpenMP tasks
 *
 * Written by Tim Mattson
 * Last modified in 2018 by Moreno Marzolla <moreno.marzolla(at)unibo.it>
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
 * This program creates a linked list whose nodes contain an integer
 * number; the nodes are then processed to compute the corresponding
 * Fibonacci number using the inefficient, recursive algorithm.
 *
 * Compile with:
 * gcc -fopenmp -std=c99 -Wall -Wpedantic omp-linked-list.c -o omp-linked-list
 *
 * Run with:
 * OMP_NUM_THREADS=2 ./omp-linked-list
 *
 ******************************************************************************/
#include <omp.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct node {
  int n;
  int fibn; /* fib(n) */
  struct node* next;
} node_t;

/**
 * Recursive computation of the n-th Fibonacci number. This is NOT the
 * most efficient way to compute a Fibonacci number, but is used here
 * since it requires a lot of work :-)
 */
int fib(int n)
{
  if (n < 2) {
    return n;
  } else {
    return fib(n - 1) + fib(n - 2);
  }
}

void process_node(node_t *p) 
{
  p->fibn = fib(p->n);
}

/**
 * Create a new list with n nodes.
 */
node_t* make_list(int n)
{
  node_t* head = NULL;
  int i;

  for (i=0; i<n; i++) {
    node_t* tmp = (node_t*)malloc(sizeof(node_t));
    tmp->next = head;
    tmp->n = rand() % 45;
    tmp->fibn = -1;
    head = tmp;
  }
  return head;
}

int main(int argc, char *argv[]) 
{
  int n_nodes = 200;
  node_t *head = NULL, *p, *tmp;

  if ( argc > 2 ) {
    fprintf(stderr, "Usage: %s [nodes]\n", argv[0]);
    return EXIT_FAILURE;
  }

  if ( argc > 1 ) {
    n_nodes = atoi(argv[1]);
  }

  p = head = make_list(n_nodes);

  const double tstart = omp_get_wtime();
#pragma omp parallel
  {
#pragma omp single
    {
      while (p != NULL) {
#pragma omp task firstprivate(p)
        process_node(p);
        p = p->next;
      }
    }
  }
  const double elapsed = omp_get_wtime() - tstart;

  /* print results and deallocate all nodes */
  p = head;
  while ( p != NULL) {
    /* printf("fib(%2d) = %d\n", p->n, p->fibn); */
    tmp = p->next;
    free(p);
    p = tmp;
  }

  fprintf(stderr, "Execution time: %f\n", elapsed);

  return EXIT_SUCCESS;
}


// vim: set nofoldenable :
