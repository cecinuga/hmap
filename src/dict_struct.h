#ifndef DICT_H_STRUCT
#define DICT_H_STRUCT

#include <stdint.h>
#include <stdint.h>
#include "hash.h"

//9679
/* ====== Dictionary constants. ====== */
#define DICT_INVALID_CELL UINT32_MAX
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

typedef enum {
    CELL_EMPTY=0,
    CELL_OCCUPIED,
    CELL_TOMBSTONE
} DictCellState;

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
    DictCellState state;
} DictEntry;

/* A simple dictionary.
 * Common operations like insert, remove and search are implemented in O(1). */
typedef struct {
    uint32_t size; // How many items are actualy storing.
    uint32_t capacity; // How many items can store.
    DoubleHashFunction hfn; // Hash function used internally

    DictEntry *entries; // List of items.
} Dict;

#endif