#ifndef DICT_H_PUBLIC
#define DICT_H_PUBLIC
#include <stdint.h>
#include "dict_struct.h"
#include "hash.h"

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
void dict_destroy(Dict *dict);

#endif