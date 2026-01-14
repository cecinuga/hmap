#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "hash.h"
#include "dict_struct.h"
#define DICT_TESTING
#include "dict_internal.h"
#include "dict_public.h"
#include "dict_err.h"

/// @brief Frees all memory associated with a dictionary entry.
/// @param entry Entry to free (must not be NULL)
/// @note Asserts if entry is NULL
/// @note Frees key, value, and string data if type is DICT_TYPE_STRING
INTERNAL void free_entry(DictEntry *entry){
    if(entry == NULL) return;
    if(entry->value && entry->value->type == DICT_TYPE_STRING)
        free(entry->value->s);

    free(entry->key);
    free(entry->value);
    entry->key = NULL,
    entry->value = NULL;
}

/// @brief Perform deep-copy of `src` into `dest`.
/// @param dest Destination value (must not be NULL)
/// @param src Source value (must not be NULL)
/// @note Asserts if either parameter is NULL
/// @note For DICT_TYPE_STRING, allocates memory that must be freed by caller
INTERNAL void dict_value_copy(DictValue *dest, DictValue *src){
    assert(dest);
    assert(src);

    dest->type = src->type;
    switch (src->type) {
    case DICT_TYPE_INT:
        dest->i = src->i;
        break;

    case DICT_TYPE_DOUBLE:
        dest->d = src->d;
        break;

    case DICT_TYPE_STRING:
        free(dest->s);
        dest->s = malloc(strlen(src->s) + 1);
        assert(dest->s);
        strcpy(dest->s, src->s);
        break;
    }
}

/// @brief Checks if a hash table cell is available (empty).
/// @param dict Dictionary pointer (must not be NULL)
/// @param cell Cell index to check (must be < dict->capacity)
/// @return 1 if cell is NULL (available), 0 otherwise
/// @note Asserts if dict is NULL or cell is out of bounds
INTERNAL int is_avaible(Dict *dict, uint32_t cell){
    assert(dict != NULL);
    assert(cell < dict->capacity);
    return dict->entries[cell].state == CELL_EMPTY ? 1: 0;
}

/// @brief Checks if dictionary is empty.
/// @param dict Dictionary pointer (must not be NULL)
/// @return 1 if empty (size == 0), 0 otherwise
/// @note Asserts if dict is NULL
INTERNAL int is_empty(Dict *dict){
    assert(dict != NULL);
    return dict->size == 0;
}

/// @brief Finds an empty slot for a given key using **double hashing**.
/// @param dict Dictionary pointer (must not be NULL)
/// @param key Key string (must not be NULL)
/// @return cell on success, DICT_INVALID_CELL otherwise
INTERNAL uint32_t get_empty_cell(Dict *dict, char *key){
    dict_clear_error();
    uint32_t i = 0;
    uint32_t cell = dict->hfn(key, i, dict->capacity);
        
    while(!is_avaible(dict, cell)){
        if(strcmp(dict->entries[cell].key, key) == 0)
            SET_ERROR_AND_RETURN(DICT_ERR_ALR_INSERTED, DICT_INVALID_CELL);
        if(i == dict->capacity){
            SET_ERROR_AND_RETURN(DICT_ERR_DICT_FULL, DICT_INVALID_CELL);
        }
        i++;
        cell = dict->hfn(key, i, dict->capacity);
    }
    assert(cell < dict->capacity);
    
    return cell;
}

/// @brief Finds the slot that store the given key using **double hashing**.
/// @param dict Dictionary pointer (must not be NULL)
/// @param key Key string (must not be NULL)
/// @return cell on success, DICT_INVALID_CELL otherwise
INTERNAL uint32_t get_key_cell(Dict *dict, char *key){
    dict_clear_error();
    uint32_t cell, i = 0;
    do {
        cell = dict->hfn(key, i, dict->capacity);
        i++;
        if(dict->entries[cell].state == CELL_EMPTY)
            SET_ERROR_AND_RETURN(DICT_ERR_NOT_FOUND, DICT_INVALID_CELL);
        if(i == dict->capacity)
            SET_ERROR_AND_RETURN(DICT_ERR_DICT_FULL, DICT_INVALID_CELL);
    } while(!(dict->entries[cell].state==CELL_OCCUPIED && strcmp(dict->entries[cell].key, key) == 0));
   
    assert(cell < dict->capacity);

    return cell;
}

/// @brief Retrieves the value associated with a given key from the dictionary.
/// @param dict Dictionary pointer (must not be NULL)
/// @param key Key string (must not be NULL)
/// @note The value is a shallow copy so will be freed with dict_destroy().
/// @return DictValue on success, NULL otherwise
INTERNAL DictEntry *get_dict_entry(Dict *dict, char *key){
    dict_clear_error();
    assert(dict);
    assert(key);

    uint32_t cell = get_key_cell(dict, key);
    if(cell == DICT_INVALID_CELL)
        SET_ERROR_AND_RETURN(dict_last_error(), NULL);
    
    DictEntry *entry = &dict->entries[cell];
    if(entry->state != CELL_OCCUPIED)
        SET_ERROR_AND_RETURN(DICT_ERR_GENERIC, NULL);

    return &dict->entries[cell];
}

/**
 * Creates a new dictionary with a fixed capacity.
 * 
 * @param capacity Number of entries the dictionary can hold (must be > 0)
 * @return Pointer to newly created Dict on success, NULL on failure
 * 
 * @note Caller owns the returned dictionary and must free it with dict_destroy()
 * @note Clears error state at start
 * @example 
 * Dict *d = dict_create(100);
 *   if (d == NULL) {
 *       fprintf(stderr, "Error: %s\n", dict_error_string(dict_last_error()));
 *   }
 */
Dict *dict_create(uint32_t capacity){
    dict_clear_error();
    if(capacity == 0) 
        SET_ERROR_AND_RETURN(DICT_ERR_INVALID_CAPACITY, NULL);

    Dict *d = malloc(sizeof(Dict));
    if(d == NULL) 
        SET_ERROR_AND_RETURN(DICT_ERR_NOMEM, NULL);

    d->size = 0;
    d->capacity = capacity;
    d->entries = calloc(capacity, sizeof(DictEntry));
    d->hfn = double_hash; // TESTING COLLISION 
    if (d->entries == NULL) {
        dict_destroy(d);
        SET_ERROR_AND_RETURN(DICT_ERR_NOMEM, NULL);
    }

    return d;
}

INTERNAL DictEntry *dict_put(Dict *dict, char *key){
    dict_clear_error();
    assert(dict != NULL);
    assert(key != NULL);
    uint32_t cell = get_empty_cell(dict, key);
    if(cell == DICT_INVALID_CELL){
        SET_ERROR_AND_RETURN(dict_last_error(), NULL);
    }
    DictEntry *entry = &dict->entries[cell];
    assert(entry->state == CELL_EMPTY);
    entry->state = CELL_OCCUPIED;

    entry->key = malloc(strlen(key));
    if(entry->key==NULL){
        free_entry(entry);
        SET_ERROR_AND_RETURN(DICT_ERR_NOMEM, NULL);
    }
    strcpy(entry->key, key);

    entry->value = malloc(sizeof(*entry->value));
    if(entry->value==NULL){
        free_entry(entry);
        SET_ERROR_AND_RETURN(DICT_ERR_NOMEM, NULL);
    }
    dict->size++;
    assert(dict->size <= dict->capacity);

    return entry;
}

/**
 * Inserts an integer value into the dictionary.
 * 
 * @param dict Dictionary to insert into (must not be NULL)
 * @param key Key string (must not be NULL, null-terminated)
 * @param val Integer value to store
 * @return 1 on success, 0 on failure
 * 
 * @note The key is copied internally; caller retains ownership of original
 * @note The value is copied internally
 * @example
 *   if (!dict_put_int(d, "age", 25)) {
 *       fprintf(stderr, "Insert failed: %s\n", 
 *               dict_error_string(dict_last_error()));
 *   }
 */
int dict_put_int(Dict *dict, char *key, int val){
    dict_clear_error();
    if(dict == NULL || key == NULL) 
        SET_ERROR_AND_RETURN(DICT_ERR_NULL_ARG, 0);

    DictEntry *entry = dict_put(dict, key);
    if(entry == NULL){
        free_entry(entry);
        return 0;
    }

    entry->value->type = DICT_TYPE_INT;
    entry->value->i = val;

    return 1;
}

/**
 * Inserts a double value into the dictionary.
 * 
 * @param dict Dictionary to insert into (must not be NULL)
 * @param key Key string (must not be NULL, null-terminated)
 * @param val Double value to store
 * @return 1 on success, 0 on failure
 * 
 * @note The key is copied internally; caller retains ownership of original
 * @note The value is copied internally
 * @example
 *   if (!dict_put_double(d, "pi", 3.14159)) {
 *       fprintf(stderr, "Insert failed: %s\n", 
 *               dict_error_string(dict_last_error()));
 *   }
 */
int dict_put_double(Dict *dict, char *key, double val){
    dict_clear_error();
    if(dict == NULL || key == NULL) 
        SET_ERROR_AND_RETURN(DICT_ERR_NULL_ARG, 0);

    DictEntry *entry = dict_put(dict, key);
    if(entry == NULL){
        free_entry(entry);
        return 0;
    }

    entry->value->type = DICT_TYPE_DOUBLE;
    entry->value->d = val;

    return 1;
}

/**
 * Inserts a string value into the dictionary.
 * 
 * @param dict Dictionary to insert into (must not be NULL)
 * @param key Key string (must not be NULL, null-terminated)
 * @param val Value string to store (must not be NULL, null-terminated)
 * @return 1 on success, 0 on failure
 * 
 * @note Both key and value are copied internally; caller retains ownership
 * @example
 *   if (!dict_put_string(d, "name", "Mario")) {
 *       fprintf(stderr, "Insert failed: %s\n", 
 *               dict_error_string(dict_last_error()));
 *   }
 */
int dict_put_string(Dict *dict, char *key, char *val){
    dict_clear_error();
    if(dict == NULL || key == NULL) 
        SET_ERROR_AND_RETURN(DICT_ERR_NULL_ARG, 0);

    DictEntry *entry = dict_put(dict, key);
    if(entry == NULL){
        free_entry(entry);
        return 0;
    }

    entry->value->type = DICT_TYPE_STRING;
    entry->value->s = malloc(strlen(val)+1);
    if(entry->value->s == NULL){
        free_entry(entry);
        SET_ERROR_AND_RETURN(DICT_ERR_NOMEM, 0);
    }
    strcpy(entry->value->s, val);

    return 1;
}

/* ========== END API INSERT IMPLEMENTATIONS ========== */


/* ========== START API UPDATE IMPLEMENTATIONS ========== */

/**
 * Update existing entry with new value.
 * 
 * @param dict Dictionary to update into (must not be NULL)
 * @param key Key string (must not be NULL, null-terminated)
 * @param val Integer value to update
 * @return 1 on success, 0 on failure
 * 
 * @note Type between old value and new value must be the same.
 */
int dict_upd_int(Dict *dict, char *key, int val){
    dict_clear_error();
    if(dict == NULL || key == NULL) 
        SET_ERROR_AND_RETURN(DICT_ERR_NULL_ARG, 0);

    DictEntry *old = get_dict_entry(dict, key);
    if(old == NULL) return 0;

    if(old->value->type != DICT_TYPE_INT)
        SET_ERROR_AND_RETURN(DICT_ERR_MIS_TYPE, 0);

    old->value->i = val;

    return 1;
}

/**
 * Update existing entry with new value.
 * 
 * @param dict Dictionary to update into (must not be NULL)
 * @param key Key string (must not be NULL, null-terminated)
 * @param val Double value to update
 * @return 1 on success, 0 on failure
 * 
 * @note Type between old value and new value must be the same.
 */
int dict_upd_double(Dict *dict, char *key, double val){
    dict_clear_error();
    if(dict == NULL || key == NULL) 
        SET_ERROR_AND_RETURN(DICT_ERR_NULL_ARG, 0);

    DictEntry *old = get_dict_entry(dict, key);
    if(old == NULL) return 0;

    if(old->value->type != DICT_TYPE_DOUBLE)
        SET_ERROR_AND_RETURN(DICT_ERR_MIS_TYPE, 0);

    old->value->d = val;

    return 1;
}

/**
 * Update existing entry with new value.
 * 
 * @param dict Dictionary to update into (must not be NULL)
 * @param key Key string (must not be NULL, null-terminated)
 * @param val String value to update
 * @return 1 on success, 0 on failure
 * 
 * @note Type between old value and new value must be the same.
 * @note The val is copied internally; caller retains ownership of original
 */
int dict_upd_string(Dict *dict, char *key, char *val){
    dict_clear_error();
    if(dict == NULL || key == NULL) 
        SET_ERROR_AND_RETURN(DICT_ERR_NULL_ARG, 0);

    DictEntry *old = get_dict_entry(dict, key);
    if(old == NULL) return 0;

    if(old->value->type != DICT_TYPE_STRING)
        SET_ERROR_AND_RETURN(DICT_ERR_MIS_TYPE, 0);

    size_t len = strlen(val) + 1;
    char *tmp = realloc(old->value->s, len);
    if (!tmp) {
        SET_ERROR_AND_RETURN(DICT_ERR_NOMEM, 0);
    }

    old->value->s = tmp;
    strcpy(old->value->s, val);

    return 1;
}

/* ========== END API UPDATE IMPLEMENTATIONS ========== */

/* ========== START API GET/TAKE IMPLEMENTATIONS ========== */

/**
 * Retrieves a value from the dictionary without removing it.
 * 
 * @param dict Dictionary to search (must not be NULL)
 * @param key Key to look up (must not be NULL, null-terminated)
 * @param out Output parameter for the value (must not be NULL)
 * @return 1 if key found and out written, 0 otherwise
 * 
 * @note If successful, the value is deep-copied into out
 * @note For DICT_TYPE_STRING, caller must free out->s after use
 * @note If key not found or error occurs, out is NOT modified
 * @example
 *   DictValue val;
 *   if (dict_get(d, "age", &val)) {
 *       printf("Age: %d\n", val.i);
 *   } else {
 *       fprintf(stderr, "Get failed: %s\n", 
 *               dict_error_string(dict_last_error()));
 *   }
 */
int dict_get(Dict *dict, char *key, DictValue *out){
    dict_clear_error(); 
    if(dict == NULL || key == NULL)
        SET_ERROR_AND_RETURN(DICT_ERR_NULL_ARG, 0);
        
    DictEntry *entry = get_dict_entry(dict, key);
    if(entry->value == NULL) return 0;
    
    dict_value_copy(out, entry->value);

    return 1;
}

/**
 * Retrieves and removes a value from the dictionary.
 * 
 * @param dict Dictionary to operate on (must not be NULL)
 * @param key Key to remove (must not be NULL, null-terminated)
 * @param out Output parameter for the value (must not be NULL)
 * @return 1 if key found, removed, and out written; 0 otherwise
 * 
 * @note If successful:
 *       - The value is deep-copied into out
 *       - The entry is removed from the dictionary
 *       - Dictionary size is decremented
 * @note For DICT_TYPE_STRING, caller must free out->s after use
 * @note If key not found or error occurs, out is NOT modified
 * @example
 *   DictValue val;
 *   if (dict_take(d, "temp", &val)) {
 *       printf("Removed value: %d\n", val.i);
 *   } else {
 *       fprintf(stderr, "Take failed: %s\n", 
 *               dict_error_string(dict_last_error()));
 *   }
 */
int dict_take(Dict *dict, char *key, DictValue *out){
    dict_clear_error();    
    if(dict == NULL || key == NULL || out == NULL)
        SET_ERROR_AND_RETURN(DICT_ERR_NULL_ARG, 0);
        
    DictEntry *entry = get_dict_entry(dict, key);
    if(entry == NULL)
        SET_ERROR_AND_RETURN(dict_last_error(), 0);

    dict_value_copy(out, entry->value);

    free_entry(entry);
    entry->state = CELL_TOMBSTONE;

    dict->size--;

    return 1;
}

/* ========== END API GET/TAKE IMPLEMENTATIONS ========== */

/**
 * Destroys the dictionary and releases all resources.
 * 
 * @param dict Dictionary to destroy (can be NULL)
 * 
 * @note Frees all entries, internal arrays, and the dictionary structure itself
 * @note After this call, the dict pointer is INVALID and must not be used
 * @note If dict is NULL, function returns immediately with no action
 * @note This function does not set error state
 * 
 * @example
 *   Dict *d = dict_create(100);
 *   // ... use dictionary ...
 *   dict_destroy(d);
 *   d = NULL;  // Good practice to NULL the pointer after destroy
 */
void dict_destroy(Dict *dict){
    if(dict == NULL) return;
    if(!is_empty(dict))
        for(uint32_t i = 0; i < dict->capacity; i++){
            if (dict->entries[i].state == CELL_EMPTY)
                continue;
            free_entry(&dict->entries[i]);
        }
        
    free(dict->entries);
    free(dict);
}
