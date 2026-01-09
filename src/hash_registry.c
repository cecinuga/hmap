#include <string.h>
#include "hash.h"

static HashEntry table[] = {
    {"ascii", hash_ascii},
    {"djb2", hash_djb2},
};

/* Returns hash function based on 'name'. 
 * Avaible hash functions are: 
 * - ascii (translate `key` param into ascii number representation. )
 * - djb2 */
HashFunction hash_get(const char *name){
    for(size_t i = 0; i < sizeof(table)/sizeof(table[0]); i++){
        if(strcmp(name, table[i].name) == 0)
            return table[i].fn;
    }

    return NULL;
}