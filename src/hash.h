#ifndef HASH_FUNCTIONS_H
#define HASH_FUNCTIONS_H
#include <stddef.h>
#include <stdint.h>

#define DICT_HASH

/* ====== Avaible hash functions ====== */
uint64_t hash_fnv1a(const char *key);
uint64_t hash_djb2(const char *key);
uint64_t bad_hash(const char *key);
uint64_t bad_hash2(const char *key);
uint32_t double_hash(char *key, uint32_t i, uint32_t size);
uint32_t double_bad_hash(char *key, uint32_t i, uint32_t size);

typedef uint32_t (*HashFunction)(const char *key);
typedef uint32_t (*DoubleHashFunction)(const char *key, uint32_t i, uint32_t size);

#endif