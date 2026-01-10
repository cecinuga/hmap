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

uint32_t double_hash(char *key, uint32_t i, uint32_t size){
    uint32_t h1 = hash_djb2(key);
    uint32_t h2 = (hash_fnv1a(key) % (size-1)) + 1;

    return (h1 + i*h2)%size; 
}