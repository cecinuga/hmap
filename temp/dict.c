#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "dict.h"
#include "hash.h"

/* Checks if a certain cell is NULL or not. */
static int is_avaible(Dict *dict, long cell){
    assert(dict->capacity > (size_t)cell); // Buffer overflow check.

    return dict->entries[cell] == NULL ? 1: 0;
}

/* Return 1 if 'dict' is empty, 0 otherwise. */
static int is_empty(Dict *dict){
    return dict->size == 0;
}

/* Free internal memory of 'entry' and 'entry' it self.
 * 'entry' can be NULL. */
static void free_entry(DictEntry *entry){
    if(!entry) return;

    if(entry->value->type==DICT_STRING)
        free(entry->value->s);
        
    free(entry->value);
    free(entry->key);
    free(entry);
}

static int dict_put(Dict *dict, char *key, DictValue *item){ 
    assert(dict);
    long k = dict->hfn(key, dict->capacity);
    assert(dict->capacity > (size_t)k); // Buffer overflow check.

    if(!is_avaible(dict, k)){
        // COLLISION HANDLING.
        return 0;
    }
    
    DictEntry *entry = malloc(sizeof(*entry));
    entry->key = key;
    entry->value = item;

    dict->size++;
    dict->entries[k] = entry;

    return 1;
}

/* Creates a new dictionary with a fixed capacity.
- `capacity` > 0
- Pointer to a valid `Dict` on success, `NULL` on allocation failure
- Caller owns the returned dictionary */
Dict *dict_create(size_t capacity){
    Dict *d = malloc(sizeof(Dict));

    d->size = 0;
    d->capacity = capacity;
    d->entries = calloc(capacity, sizeof(DictValue*));
    d->hfn = hash_get(DICTIONARY_HASH_FUNCTION);

    return d;
}

/* Inserts a key → value pair into the dictionary.

- `dict` must not be `NULL`
- `key` must not be `NULL` and must be a null-terminated string
- The key is copied internally
- The value is copied internally
- If a collision occurs, insertion fails
- Returns `1` on successful insertion, `0` on failure (collision, invalid input, allocation failure)
- The caller retains ownership of `key` */
int dict_put_int(Dict *dict, char *key, int val){
    assert(dict);

    DictValue *dval = malloc(sizeof(*dval));
    dval->type = DICT_INT;
    dval->i = val;

    int res = dict_put(dict, key, dval);
    if(!res) free(dval);

    return res;
}

/* Inserts a key → value pair into the dictionary.

- `dict` must not be `NULL`
- `key` must not be `NULL` and must be a null-terminated string
- The key is copied internally
- The value is copied internally
- If a collision occurs, insertion fails
- Returns `1` on successful insertion, `0` on failure (collision, invalid input, allocation failure)
- The caller retains ownership of `key` */
int dict_put_double(Dict *dict, char *key, double val){
    assert(dict);

    DictValue *dval = malloc(sizeof(*dval));
    dval->type = DICT_DOUBLE;
    dval->d = val;

    int res = dict_put(dict, key, dval);
    if(!res) free(dval);

    return res;
}

/* Inserts a key → value pair into the dictionary.

- `dict` must not be `NULL`
- `key` must not be `NULL` and must be a null-terminated string
- The key is copied internally
- The value is copied internally
- If a collision occurs, insertion fails
- Returns `1` on successful insertion, `0` on failure (collision, invalid input, allocation failure)
- The caller retains ownership of `key` */
int dict_put_string(Dict *dict, char *key, char *val){
    assert(dict);

    char *newkey = malloc(sizeof(char*)); 
    strcpy(newkey, key); // copy the key so caller retains ownership of the original key string
    
    DictValue *dval = malloc(sizeof(*dval));
    dval->type = DICT_STRING;
    dval->s = val;

    int res = dict_put(dict, newkey, dval);
    if(!res) free(dval);

    return res;
}

/* Retrieves the value associated with a key without removing it.
- `dict`, `key`, `out` must not be `NULL`
- If the key exists, `out` is written
- If the key does not exist, `out` is not modified
- Returns `1` if the key was found, `0` if the key was not found or on error*/
int dict_get(Dict *dict, char *key, DictValue *out){
    long k = dict->hfn(key, dict->capacity);

    if (!dict->entries[k])
        return 0;

    if (strcmp(dict->entries[k]->key, key) != 0)
        return 0;

    *out = *dict->entries[k]->value;
    return 1;
}

/* Retrieves and removes the value associated with a key.
- `dict`, `key`, `out` must not be `NULL`
- If the key exists:
- - The value is copied into `out`
- - The entry is removed from the dictionary
- If the key does not exist:
- - No state change occurs
- Returns `1` if the element was removed, `0` if the key was not found or on error*/
int dict_take(Dict *dict, char *key, DictValue *out){
    long k = dict->hfn(key, dict->capacity);

    if (!dict->entries[k])
        return 0;

    if (strcmp(dict->entries[k]->key, key) != 0)
        return 0;

    DictEntry *entry = dict->entries[k];
    *out = *entry->value;

    free_entry(entry);

    dict->entries[k] = NULL;
    dict->size--;

    return 1;
}

/* Removes all entries from the dictionary.
- Frees all internal entries
- The dictionary remains valid and reusable */
void dict_cleanup(Dict *dict){
    assert(dict);
    if(is_empty(dict)) return;

    for(size_t i = 0; i < dict->capacity; i++){
        if (!dict->entries[i])
            continue;

        free(dict->entries[i]);
        dict->entries[i] = NULL;
    }
    
    dict->size = 0;
}

/* Destroys the dictionary and releases all associated resources.
- Frees all internal memory
- After this call, the dictionary pointer is invalid*/
void dict_destroy(Dict *dict){        
    assert(dict);
    dict_cleanup(dict);

    free(dict->entries);
    free(dict);
}


