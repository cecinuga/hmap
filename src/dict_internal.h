#ifndef DICT_H_INTERNAL
#define DICT_H_INTERNAL
#include "dict_struct.h"

#ifdef DICT_TESTING
#define INTERNAL
#else
#define INTERNAL static
#endif

INTERNAL void dict_value_copy(DictValue *dest, const DictValue *src);
INTERNAL int is_avaible(Dict *dict, uint32_t cell);
INTERNAL int is_empty(Dict *dict);
INTERNAL void free_entry(DictEntry *entry);
INTERNAL uint32_t get_empty_cell(Dict *dict, char *key);
INTERNAL uint32_t get_key_cell(Dict *dict, char *key);
INTERNAL DictValue *get_dict_value(Dict *dict, char *key);
INTERNAL int dict_put(Dict *dict, char *key, DictValue *item);

#endif