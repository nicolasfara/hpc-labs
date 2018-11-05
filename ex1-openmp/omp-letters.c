/* */
/****************************************************************************
 *
 * omp-letters.c - Count occurrences of letters 'a'..'z' from stdin
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
 * --------------------------------------------------------------------------
 *
 * Compute and print the frequencies of the 26 alphabetic characters
 * 'a'..'z' on the input stream read from stdin. Uppercase characters
 * are converted to lowercase; all other characters are ignored.
 *
 * Compile with:
 * gcc -fopenmp -Wall -Wpedantic omp-letters.c -o omp-letters
 *
 * Run with:
 * ./omp-letters < the-war-of-the-worlds.txt
 *
 ****************************************************************************/
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

/**
 * Count occurrences of letters 'a'..'z' in |text|; uppercase
 * characters are transformed to lowercase, and all other symbols are
 * ignored. |len| is the length of |text|, that must be a
 * zero-terminated string. |hist| will be filled with the computed
 * counts. Returns the total number of letters found.
 */
int make_hist( const char *text, int len, int hist[26] )
{
    int nlet = 0; /* total number of alphabetic characters processed */
    int i, j;
    /* [TODO] Parallelize this function */
    /* Reset histogram */
    for (j=0; j<26; j++) {
        hist[j] = 0;
    }
    /* Count occurrences */
#pragma omp parallel for default(none) shared(nlet, hist) firstprivate(len, text)
    for (i=0; i<len; i++) {
        char c = text[i];
        if (isalpha(c)) {
#pragma omp atomic
            nlet++;
#pragma omp atomic
            hist[ tolower(c) - 'a' ]++;
        }
    }
    return nlet;
}

/**
 * Print frequencies
 */
void print_hist( int hist[26] )
{
    int i;
    int nlet = 0;
    for (i=0; i<26; i++) {
        nlet += hist[i];
    }
    for (i=0; i<26; i++) {
        printf("%c : %8d (%6.2f%%)\n", 'a'+i, hist[i], 100.0f*hist[i]/nlet);
    }
    printf("    %8d total\n", nlet);
}

int main( void )
{
    int hist[26];
    const size_t size = 5*1024*1024;
    char *text = (char*)malloc(size); assert(text != NULL);
    size_t len = fread(text, 1, size-1, stdin);
    text[len] = '\0'; /* terminate text */
    const double tstart = omp_get_wtime();    
    make_hist(text, len, hist);
    const double elapsed = omp_get_wtime() - tstart;  
    print_hist(hist);
    printf("Elapsed time: %f\n", elapsed);
    return 0;
}

// vim: set nofoldenable ts=4 sw=4 :
