#include <string.h>
#include "hash.h"

static HashEntry table[] = {
    {"ascii", hash_ascii},
    {"djb2", hash_djb2},
};

/* It returns a hash function based on 'name'. */
HashFunction hash_get(const char *name){
    for(size_t i = 0; i < sizeof(table)/sizeof(table[0]); i++){
        if(strcmp(name, table[i].name) == 0)
            return table[i].fn;
    }

    return NULL;
}