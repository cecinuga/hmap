#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "dict.h"
#include "hash.h"

/* ========== ERROR HANDLING IMPLEMENTATION ========== */

// Thread-local storage per l'ultimo errore
static _Thread_local DictError g_last_error = DICT_OK;

DictError dict_last_error(void) {
    return g_last_error;
}

void dict_clear_error(void) {
    g_last_error = DICT_OK;
}

// Macro helper per settare errore e ritornare
#define SET_ERROR_AND_RETURN(err, retval) { \
    g_last_error = (err); \
    return (retval); \
}

const char *dict_error_string(DictError err) {
    switch (err) {
        case DICT_OK:
            return "Success";
        case DICT_ERR_NULL_ARG:
            return "NULL argument provided";
        case DICT_ERR_NOMEM:
            return "Memory allocation failed";
        case DICT_ERR_COLLISION:
            return "Hash collision occurred";
        case DICT_ERR_NOT_FOUND:
            return "Key not found";
        case DICT_ERR_INVALID_CAPACITY:
            return "Invalid capacity (must be > 0)";
        default:
            return "Unknown error";
    }
}


/* ========== PRIVATE HELPERS ========== */

/// @brief Perform deep-copy of `src` into `dest`.
/// @param dest Destination value (must not be NULL)
/// @param src Source value (must not be NULL)
/// @note Asserts if either parameter is NULL
/// @note For DICT_TYPE_STRING, allocates memory that must be freed by caller
static void dict_value_copy(DictValue *dest, const DictValue *src){
    assert(dest && src);

    dest->type = src->type;

    switch (src->type) {
    case DICT_TYPE_INT:
        dest->i = src->i;
        break;

    case DICT_TYPE_DOUBLE:
        dest->d = src->d;
        break;

    case DICT_TYPE_STRING:
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
static int is_avaible(Dict *dict, unsigned long cell){
    assert(dict != NULL);
    assert(cell < dict->capacity);
    return dict->entries[cell] == NULL ? 1: 0;
}

/// @brief Checks if dictionary is empty.
/// @param dict Dictionary pointer (must not be NULL)
/// @return 1 if empty (size == 0), 0 otherwise
/// @note Asserts if dict is NULL
static int is_empty(Dict *dict){
    assert(dict != NULL);
    return dict->size == 0;
}

/// @brief Frees all memory associated with a dictionary entry.
/// @param entry Entry to free (must not be NULL)
/// @note Asserts if entry is NULL
/// @note Frees key, value, and string data if type is DICT_TYPE_STRING
static void free_entry(DictEntry *entry){
    assert(entry != NULL);
    if(entry->value->type==DICT_TYPE_STRING)
        free(entry->value->s);
        
    free(entry->value);
    free(entry->key);
    free(entry);
}

/// @brief Internal function to insert a key-value pair into the dictionary.
/// @param dict Dictionary pointer (must not be NULL)
/// @param key Key string (must not be NULL)
/// @param item Value to insert (must not be NULL)
/// @return 1 on success, 0 on failure
/// @note Asserts if any parameter is NULL
/// @note Clears error state at start
static int dict_put(Dict *dict, char *key, DictValue *item){   
    dict_clear_error();
    assert(dict != NULL);
    assert(dict->hfn != NULL);  
    assert(key != NULL);
    assert(item != NULL);
    
    unsigned long cell = dict->hfn(key, dict->capacity);
    assert(dict->capacity > cell); // Buffer overflow check.

    if(!is_avaible(dict, cell)){
        // COLLISION HANDLING.
        SET_ERROR_AND_RETURN(DICT_ERR_COLLISION, 0);
    }
    
    DictEntry *entry = malloc(sizeof(*entry));
    if (entry == NULL) SET_ERROR_AND_RETURN(DICT_ERR_NOMEM, 0);
    
    entry->key = malloc(strlen(key)+1);
    if(entry->key == NULL) {
        free(entry);
        SET_ERROR_AND_RETURN(DICT_ERR_NOMEM, 0);
    }

    entry->value = calloc(1, sizeof(*entry->value));
    if(entry->value == NULL) {
        free(entry->key);
        free(entry);
        SET_ERROR_AND_RETURN(DICT_ERR_NOMEM, 0);
    }

    strcpy(entry->key, key);
    dict_value_copy(entry->value, item);

    assert(dict->entries[cell] == NULL);
    
    dict->size++;
    dict->entries[cell] = entry;

    assert(dict->size <= dict->capacity);

    return 1;
}

/* ========== PUBLIC API IMPLEMENTATION ========== */

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
Dict *dict_create(size_t capacity){
    dict_clear_error();
    if(capacity == 0) 
        SET_ERROR_AND_RETURN(DICT_ERR_INVALID_CAPACITY, NULL);

    Dict *d = malloc(sizeof(Dict));
    if(d == NULL) 
        SET_ERROR_AND_RETURN(DICT_ERR_NOMEM, NULL);

    d->size = 0;
    d->capacity = capacity;
    d->hfn = hash_get(DICTIONARY_HASH_FUNCTION);
    d->entries = calloc(capacity, sizeof(DictEntry*));
    if (d->entries == NULL) {
        dict_destroy(d);
        SET_ERROR_AND_RETURN(DICT_ERR_NOMEM, NULL);
    }

    return d;
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

    DictValue *dval = malloc(sizeof(*dval));
    if(dval == NULL) 
        SET_ERROR_AND_RETURN(DICT_ERR_NOMEM, 0);

    dval->type = DICT_TYPE_INT;
    dval->i = val;

    int res = dict_put(dict, key, dval);
    free(dval);

    return res;
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
        SET_ERROR_AND_RETURN(DICT_ERR_NOMEM, 0);

    DictValue *dval = malloc(sizeof(*dval));
    if(dval == NULL)
        SET_ERROR_AND_RETURN(DICT_ERR_NOMEM, 0);

    dval->type = DICT_TYPE_DOUBLE;
    dval->d = val;

    int res = dict_put(dict, key, dval);
    free(dval);

    return res;
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
    if(dict == NULL || key == NULL || val == NULL) 
        SET_ERROR_AND_RETURN(DICT_ERR_NULL_ARG, 0);

    DictValue *dval = malloc(sizeof(*dval));
    if(dval == NULL) 
        SET_ERROR_AND_RETURN(DICT_ERR_NOMEM, 0);

    dval->type = DICT_TYPE_STRING;
    dval->s = strdup(val);
    if(dval->s == NULL) {
        free(dval);  
        SET_ERROR_AND_RETURN(DICT_ERR_NOMEM, 0);
    }

    int res = dict_put(dict, key, dval);
    free(dval->s);
    free(dval);   

    return res;
}

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
    if(dict == NULL || key == NULL || out == NULL)
        SET_ERROR_AND_RETURN(DICT_ERR_NULL_ARG, 0);

    unsigned long cell = dict->hfn(key, dict->capacity);
    assert(dict->capacity > cell);

    if (is_avaible(dict, cell))
        SET_ERROR_AND_RETURN(DICT_ERR_NOT_FOUND, 0); // not found
    if (strcmp(dict->entries[cell]->key, key) != 0) 
        SET_ERROR_AND_RETURN(DICT_ERR_COLLISION, 0); // collision

    dict_value_copy(out, dict->entries[cell]->value);

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

    unsigned long cell = dict->hfn(key, dict->capacity);
    assert(dict->capacity > cell);

    if (is_avaible(dict, cell))
        SET_ERROR_AND_RETURN(DICT_ERR_NOT_FOUND, 0); // not found
    if (strcmp(dict->entries[cell]->key, key) != 0)
        SET_ERROR_AND_RETURN(DICT_ERR_COLLISION, 0); // collision

    DictEntry *entry = dict->entries[cell];
    dict_value_copy(out, dict->entries[cell]->value);

    free_entry(entry);

    dict->entries[cell] = NULL;
    dict->size--;

    return 1;
}

/**
 * Removes all entries from the dictionary.
 * 
 * @param dict Dictionary to clear (can be NULL)
 * 
 * @note Frees all internal entries and their associated memory
 * @note The dictionary remains valid and reusable after cleanup
 * @note Size is reset to 0
 * @note Capacity remains unchanged
 * @note If dict is NULL, function returns immediately with no action
 * @note This function does not set error state
 * 
 * @example
 *   dict_cleanup(d);  // Dictionary is now empty but still usable
 *   dict_put_int(d, "new_key", 42);  // Can insert again
 */
void dict_cleanup(Dict *dict){
    if(dict == NULL) return;

    for(size_t i = 0; i < dict->capacity; i++){
        if (!dict->entries[i])
            continue;

        free_entry(dict->entries[i]);
        dict->entries[i] = NULL;
    }
    
    dict->size = 0;
}

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

    dict_cleanup(dict);

    free(dict->entries);
    free(dict);
}


