/* */
/****************************************************************************
 *
 * omp-brute-force.c - Brute-force password cracking
 *
 * Written in 2017 by Moreno Marzolla <moreno.marzolla(at)unibo.it>
 * Modified in 2018 by Moreno Marzolla
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
 * Skeleton program containing an encrypted message that must be
 * decrypted by brute-force search of the key space using OpenMP.  The
 * encryption key is known to be a sequence of 8 ASCII numeric
 * characters; therefore, the key space is "00000000" - "99999999". It
 * is also known that the correctly decrypted message is a sequence of
 * printable characters that starts with "0123456789" (no quotes); the
 * rest of the plaintext is a quote from an old movie.
 *
 * Compile with:
 * gcc -std=c99 -Wall -Wpedantic -fopenmp omp-brute-force.c -o omp-brute-force
 *
 * Run with:
 * ./omp-brute-force
 *
 ****************************************************************************/
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <rpc/des_crypt.h>

/* Decrypt cyphertext |enc| of length |n| bytes into buffer |dec|
   using |key|. Since this function uses ecb_crypt() to decrypt the
   message, the key must be exactly 8 bytes long. Note that the
   encrypted message, decrypted messages and key are binary blobs;
   hence, they are not required to be zero-terminated. 

   This function should not be modified. */
void decrypt(const char* enc, char* dec, int n, const char* key)
{
    int err;
    char keytmp[8];
    assert( n % 8 == 0 );   /* ecb_crypt requires the data length to be a multiple of 8 */
    memcpy(keytmp, key, 8); /* copy the key to a temporary buffer */
    memcpy(dec, enc, n);    /* copy the encrypted message to the decription buffer */
    //des_setparity(keytmp);  /* set key parity */
    err = ecb_crypt(keytmp, dec, n, DES_DECRYPT | DES_SW);
    assert( DESERR_NONE == err );
}


int main( int argc, char *argv[] )
{
    /* encrypted message */
    const char enc[] = {
        -109, 27, 102, 85, -20, -119, -96, -38,
        46, 63, -57, -83, -37, -83, 91, 41,
        -122, -18, 118, -55, -39, -117, 79, 8,
        -18, 46, -68, -20, 10, -18, -113, 17,
        26, -49, 23, -4, -45, -113, 78, -27,
        -29, -97, -54, 26, 1, 118, -123, 66,
        -28, -28, -83, -69, 121, -68, 99, -112,
        97, -120, 11, -56, -108, 82, -18, 67
    }; 
    const int msglen = sizeof(enc);
    const char check[] = "0123456789"; /* the correctly decrypted message starts with these characters */
    
    /* How to use a key to decrypt the message */
    char key[9]; /* sprintf will output the trailing \0, so we need one byte more for the key */
    int k = 132; /* numeric value of the key to try */
    char* out = (char*)malloc(msglen); /* where to put the decrypted message */
    snprintf(key, 9, "%08d", k);
    decrypt(enc, out, msglen, key);
    /* Now |out| contains the decrypted bytes; if the decryption key
       is not the one used in the encryption process, |out| will
       contain "random" garbage */
    if ( 0 == memcmp(out, check, strlen(check)) ) {
        printf("Key found: %s\n", key);
    } else {
        printf("Key %s not valid\n", key);
    }
    free(out);
    return 0;
}

// vim: set nofoldenable :
