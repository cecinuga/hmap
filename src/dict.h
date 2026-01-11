#ifndef DICT_H
#define DICT_H
#include <stddef.h>
#include <stdint.h>
#include "hash.h"

/* ====== Dictionary constants. ====== */
#define INVALID_CELL UINT32_MAX
#define DICT_CAP 701
#define DICT_HASH_PRIMARY "djb2"
#define DICT_HASH_SECONDARY "fnv1a"

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
    uint32_t size; // How many items are actualy storing.
    uint32_t capacity; // How many items can store.
    DoubleHashFunction hfn; // Hash function used internally

    DictEntry **entries; // List of items.
} Dict;

/* ====== Dictionary API ====== */

Dict *dict_create(uint32_t capacity);
int dict_put_int(Dict *dict, char *key, int val);
int dict_put_double(Dict *dict, char *key, double val);
int dict_put_string(Dict *dict, char *key, char *val);
int dict_upd_int(Dict *dict, char *key, int val);
int dict_upd_double(Dict *dict, char *key, double val);
int dict_upd_string(Dict *dict, char *key, char *val);
int dict_take(Dict *dict, char *key, DictValue *out);
int dict_get(Dict *dict, char *key, DictValue *out);
void dict_cleanup(Dict *dict);
void dict_destroy(Dict *dict);

#endif