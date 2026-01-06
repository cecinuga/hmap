#include <stdio.h>
#include "utils.h"
#include "hash.h"

/* Hash function, a good hash function satisfies the uniform hash hypothesis.
 * So every key has the same probability of being associated to a certain cell. */
long hash_ascii(const char *key, const size_t size){
    long k = string_to_ascii_long(key);

    return k % size;
}

long hash_djb2(const char *key, const size_t size) {
    long h = HASH_MAGIC_N;

    for (size_t i = 0; i < size; i++)
        h = ((h << 5) + h) + key[i];

    return h % size;
}