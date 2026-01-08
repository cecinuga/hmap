#ifndef HASH_FUNCTIONS_H
#define HASH_FUNCTIONS_H
#include <stddef.h>

#define HASH_MAGIC_N 5381

/* ====== Avaible hash functions ====== */
unsigned long hash_ascii(const char *key, const size_t size);
unsigned long hash_djb2(const char *key, const size_t size);

/* ====== Common hash function signs. ====== */
typedef long (*HashFunction)(const char *key, const size_t size);

/* ====== Utility ====== */
typedef struct {
    const char *name;
    HashFunction fn;
} HashEntry;

/* It returns a hash function based on 'name'. */
HashFunction hash_get(const char *name);

#endif