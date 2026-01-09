#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "dict.h"
#include "hash.h"

/// @brief Perform deep-copy of `dest` into `src`.
/// @param dest 
/// @param src 
static void dict_value_copy(DictValue *dest, const DictValue *src){
    assert(dest && src);

    dest->type = src->type;

    switch (src->type) {
    case DICT_INT:
        dest->i = src->i;
        break;

    case DICT_DOUBLE:
        dest->d = src->d;
        break;

    case DICT_STRING:
        dest->s = malloc(strlen(src->s) + 1);
        assert(dest->s);
        strcpy(dest->s, src->s);
        break;
    }
}

/* Checks if a certain cell is NULL or not. */
static int is_avaible(Dict *dict, unsigned long cell){
    assert(dict != NULL);
    assert(cell < dict->capacity);
    return dict->entries[cell] == NULL ? 1: 0;
}

/* Return 1 if 'dict' is empty, 0 otherwise. */
static int is_empty(Dict *dict){
    assert(dict != NULL);
    return dict->size == 0;
}

/* Free internal memory of 'entry' and 'entry' it self.
 * 'entry' can be NULL. */
static void free_entry(DictEntry *entry){
    assert(entry != NULL);
    if(entry->value->type==DICT_STRING)
        free(entry->value->s);
        
    free(entry->value);
    free(entry->key);
    free(entry);
}

static int dict_put(Dict *dict, char *key, DictValue *item){   
    assert(dict != NULL);
    assert(dict->hfn != NULL);  

    unsigned long cell = dict->hfn(key, dict->capacity);
    assert(dict->capacity > cell); // Buffer overflow check.

    if(!is_avaible(dict, cell)){
        // COLLISION HANDLING.
        return 0;
    }
    
    DictEntry *entry = malloc(sizeof(*entry));
    if (entry == NULL) return 0;
    
    entry->key = malloc(strlen(key)+1);
    if(entry->key == NULL) {
        free(entry);
        return 0;
    }

    entry->value = calloc(1, sizeof(*entry->value));
    if(entry->value == NULL) {
        free(entry->key);
        free(entry);
        return 0;
    }

    strcpy(entry->key, key);
    dict_value_copy(entry->value, item);

    assert(dict->entries[cell] == NULL);
    
    dict->size++;
    dict->entries[cell] = entry;

    assert(dict->size <= dict->capacity);

    return 1;
}

/* Creates a new dictionary with a fixed capacity.
 * - `capacity` > 0
 * - Pointer to a valid `Dict` on success, `NULL` on allocation failure
 * - Caller owns the returned dictionary. */
Dict *dict_create(size_t capacity){
    if(capacity == 0) return NULL;

    Dict *d = malloc(sizeof(Dict));
    if(d == NULL) return NULL;

    d->size = 0;
    d->capacity = capacity;
    d->hfn = hash_get(DICTIONARY_HASH_FUNCTION);
    d->entries = calloc(capacity, sizeof(DictEntry*));
    if (d->entries == NULL) {
        dict_destroy(d);
        return NULL;
    }

    return d;
}

/* Inserts a key → value pair into the dictionary.
 * - `dict` must not be `NULL`
 * - `key` must not be `NULL` and must be a null-terminated string
 * - The key is copied internally
 * - The value is copied internally
 * - If a collision occurs, insertion fails
 * - Returns `1` on successful insertion, `0` on failure (collision, invalid input, allocation failure)
 * - The caller retains ownership of `key`. */
int dict_put_int(Dict *dict, char *key, int val){
    if(dict == NULL || key == NULL) return 0;

    DictValue *dval = malloc(sizeof(*dval));
    if(dval == NULL) return 0;

    dval->type = DICT_INT;
    dval->i = val;

    int res = dict_put(dict, key, dval);
    free(dval);

    return res;
}

/* Inserts a key → value pair into the dictionary.
 * - `dict` must not be `NULL`
 * - `key` must not be `NULL` and must be a null-terminated string
 * - The key is copied internally
 * - The value is copied internally
 * - If a collision occurs, insertion fails
 * - Returns `1` on successful insertion, `0` on failure (collision, invalid input, allocation failure)
 * - The caller retains ownership of `key`. */
int dict_put_double(Dict *dict, char *key, double val){
    if(dict == NULL || key == NULL) return 0;

    DictValue *dval = malloc(sizeof(*dval));
    if(dval == NULL) return 0;

    dval->type = DICT_DOUBLE;
    dval->d = val;

    int res = dict_put(dict, key, dval);
    free(dval);

    return res;
}

/* Inserts a key → value pair into the dictionary.
 * - `dict` must not be `NULL`
 * - `key` must not be `NULL` and must be a null-terminated string
 * - The key is copied internally
 * - The value is copied internally
 * - If a collision occurs, insertion fails
 * - Returns `1` on successful insertion, `0` on failure (collision, invalid input, allocation failure)
 * - The caller retains ownership of `key`. */
int dict_put_string(Dict *dict, char *key, char *val){
    if(dict == NULL || key == NULL || val == NULL) return 0;

    DictValue *dval = malloc(sizeof(*dval));
    if(dval == NULL) return 0;

    dval->type = DICT_STRING;
    dval->s = strdup(val);
    if(dval->s == NULL) {
        free(dval);  
        return 0;
    }

    int res = dict_put(dict, key, dval);
    free(dval->s);
    free(dval);   

    return res;
}

/* Retrieves the value associated with a key without removing it.
 * - `dict`, `key`, `out` must be `non-NULL`
 * - If the key exists, `out` is written
 * - If the key does not exist, `out` is not modified
 * - Returns `1` if `out` is written
 * - Returns `0` if the key was not found or failure (collision, invalid input)*/
int dict_get(Dict *dict, char *key, DictValue *out){
    if(dict == NULL || key == NULL || out == NULL) return 0;

    unsigned long k = dict->hfn(key, dict->capacity);
    assert(dict->capacity > k);

    if (!dict->entries[k]) return 0; // empty cell
    if (strcmp(dict->entries[k]->key, key) != 0) return 0; // collision

    dict_value_copy(out, dict->entries[k]->value);

    return 1;
}

/* Retrieves and removes the value associated with a key.
 * - `dict`, `key`, `out` must be `non-NULL`
 * - If the key exists:
 * - - The value is copied into `out`
 * - - The entry is removed from the dictionary
 * - If the key does not exist, out is not modified
 * - Returns `1` if the element was removed
 * - Returns `0` if the key was not found or failure (collision, invalid input)*/
int dict_take(Dict *dict, char *key, DictValue *out){
    if(dict == NULL || key == NULL || out == NULL) return 0;

    unsigned long k = dict->hfn(key, dict->capacity);
    assert(dict->capacity > k);

    if (!dict->entries[k]) return 0; // empty cell
    if (strcmp(dict->entries[k]->key, key) != 0) return 0; // collision

    DictEntry *entry = dict->entries[k];
    dict_value_copy(out, dict->entries[k]->value);

    free_entry(entry);

    dict->entries[k] = NULL;
    dict->size--;

    return 1;
}

/* Removes all entries from the dictionary.
 * - Frees all internal entries
 * - The dictionary remains valid and reusable 
 * - If `dict` is NULL nothing happen */
void dict_cleanup(Dict *dict){
    if(dict == NULL || is_empty(dict)) return;

    for(size_t i = 0; i < dict->capacity; i++){
        if (!dict->entries[i])
            continue;

        free_entry(dict->entries[i]);
        dict->entries[i] = NULL;
    }
    
    dict->size = 0;
}

/* Destroys the dictionary and releases all associated resources.
 * - Frees all internal memory
 * - After this call, the dictionary pointer is invalid
 * - If `dict` is NULL nothing happen */
void dict_destroy(Dict *dict){        
    if(dict == NULL) return;

    dict_cleanup(dict);

    free(dict->entries);
    free(dict);
}


