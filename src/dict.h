#ifndef DICT_H
#define DICT_H

#include <stddef.h>
#include "hash.h"

/* ========== ERROR HANDLING ========== */

typedef enum {
    DICT_OK = 0,              // Successo
    DICT_ERR_NULL_ARG,        // Argomento NULL passato
    DICT_ERR_NOMEM,           // Allocazione memoria fallita
    DICT_ERR_COLLISION,       // Collisione hash
    DICT_ERR_NOT_FOUND,       // Chiave non trovata
    DICT_ERR_INVALID_CAPACITY // Capacity = 0 in dict_create
} DictError;

// Ottiene l'ultimo errore (thread-safe)
DictError dict_last_error(void);

// Resetta l'ultimo errore a DICT_OK
void dict_clear_error(void);

// Converte codice errore in stringa leggibile
const char *dict_error_string(DictError err);

/* ====== Dictionary constants. ====== */
#define DICTIONARY_HASH_FUNCTION "djb2"

/* ====== Dictionary struct ====== */

/* Valid types Dict can store. */
typedef enum {
    DICT_TYPE_INT, // int
    DICT_TYPE_DOUBLE, // double
    DICT_TYPE_STRING // char*
} DictType;

typedef struct {
    DictType type;
    union {
        int i;
        double d;
        char *s;
    };
} DictValue;

typedef struct {
    char *key;
    DictValue *value;
} DictEntry;

/* A simple dictionary.
 * Common operations like insert, remove and search are implemented in O(1). */
typedef struct {
    size_t size; // How many items are actualy storing.
    size_t capacity; // How many items can store.
    HashFunction hfn; // Hash function for calculating 'item' position based on 'key'.

    DictEntry **entries; // List of items.
} Dict;

/* ====== Dictionary utils ====== */

Dict *dict_create(size_t capacity);
int dict_put_int(Dict *dict, char *key, int val);
int dict_put_double(Dict *dict, char *key, double val);
int dict_put_string(Dict *dict, char *key, char *val);
int dict_take(Dict *dict, char *key, DictValue *out);
int dict_get(Dict *dict, char *key, DictValue *out);
void dict_cleanup(Dict *dict);
void dict_destroy(Dict *dict);

#endif