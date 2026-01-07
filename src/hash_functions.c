#include <stdio.h>
#include "utils.h"
#include "hash.h"

/* Hash function, a good hash function satisfies the uniform hash hypothesis.
 * So every key has the same probability of being associated to a certain cell. */
long hash_ascii(const char *key, const size_t size){
    long k = string_to_ascii_long(key);

    return k % size;
}

long hash_djb2(const char *str, const size_t size){
    unsigned long hash = HASH_MAGIC_N;
    int c;

    while ((c = (unsigned char)*str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash % size;
}