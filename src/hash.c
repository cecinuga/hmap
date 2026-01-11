#include <stdio.h>
#include <stdint.h>
#include "utils.h"
#include "hash.h"

uint64_t hash_djb2(const char *str){
    uint64_t hash = 5381;

    int c;

    while ((c = (unsigned char)*str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash;
}

uint64_t hash_fnv1a(const char *key) {
    uint64_t hash = 14695981039346656037ULL; // offset basis
    while (*key) {
        hash ^= (unsigned char)*key++;
        hash *= 1099511628211ULL; // FNV prime
    }
    return hash;
}

uint64_t bad_hash(const char *key) {
    (void)key;
    return 42;
}

uint64_t bad_hash2(const char *key) {
    uint64_t h = 0;
    while (*key) h += *key++;
    return h | 1; // garantisce passo ≠ 0
}

uint32_t double_hash(const char *key, uint32_t i, uint32_t size){
    uint32_t h1 = hash_djb2(key);
    uint32_t h2 = (hash_fnv1a(key) % (size-1)) + 1;

    return (h1 + i*h2)%size; 
}

uint32_t double_bad_hash(const char *key, uint32_t i, uint32_t size){
    uint32_t h1 = bad_hash(key);
    uint32_t h2 = (bad_hash2(key) % (size-1)) + 1;

    return (h1 + i*h2)%size; 
}