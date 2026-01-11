#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "dict.h"
#include "dict_err.h"

#define DICT_MAX_SIZE 104

int probing_test(){
    int *visited = calloc(DICT_CAP, sizeof(int));

    for (uint32_t i = 0; i < DICT_CAP; i++) {
        char key[16];
        sprintf(key, "key_%d", i);

        uint32_t cell = double_hash(key, i, DICT_CAP);

        printf("key: %s, cell: %zu, state: %d\n", key, cell, visited[cell]);
        assert(!visited[cell]);

        visited[cell] = 1;
    }

    return 0;
}

int functional_test(){
    Dict *dict = dict_create(DICT_CAP);
    if (!dict) return 1;

    printf("start inserting entries...\n");
    for (int i = 0; i < DICT_MAX_SIZE; i++) {
      char key[16];
      sprintf(key, "key_%d", i);
      int res = dict_put_int(dict, key, i);
      if(!res){
        perror(dict_error_string(dict_last_error()));
        dict_destroy(dict);
        return 1;
      }
    
      assert(dict->size <= dict->capacity);
      assert(dict->size == (uint32_t)i+1);
    }

    printf("start updating entries...\n");
    for (int i = 0; i < DICT_MAX_SIZE; i++) {
      char key[16];
      sprintf(key, "key_%d", i);
      int res = dict_upd_int(dict, key, i+1);
      if(!res){
        perror(dict_error_string(dict_last_error()));
        dict_destroy(dict);
        return 1;
      }

      assert(dict->size == DICT_MAX_SIZE);
    }
    
    printf("dict size is: %d\n", dict->size);
    assert(dict->size == DICT_MAX_SIZE);

    printf("start getting entries...\n");
    for (int i = 0; i < DICT_MAX_SIZE; i++) {
      char key[16];
      sprintf(key, "key_%d", i);

      DictValue v;
      int res = dict_get(dict, key, &v);
      if(!res){
        perror(dict_error_string(dict_last_error()));
        dict_destroy(dict);
        return 1;
      }
      assert(v.i == i+1);
      assert(dict->size == DICT_MAX_SIZE);
    }

    printf("start taking entries...\n");
    for (int i = 0; i < DICT_MAX_SIZE; i++) {
      char key[16];
      sprintf(key, "key_%d", i);

      DictValue v;
      
      int res = dict_take(dict, key, &v);
      if(!res){
        printf("-------key: %s\n", key);
        perror(dict_error_string(dict_last_error()));
        dict_destroy(dict);
        return 1;
      }

      assert(dict->size == (uint32_t)DICT_MAX_SIZE-(i+1));
      assert(v.i == i+1);
    }

    printf("dict size is: %d\n", dict->size);
    assert(dict->size == 0);
    printf("\n[+] .....SUCCESS.....\n");
    dict_destroy(dict);

    return 0;
}

