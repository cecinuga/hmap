#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "test.h"
#define DICT_TESTING
#include "dict_struct.h"
#include "dict_internal.h"
#include "dict_err.h"
#include "dict_public.h"

int collision_test(){
    Dict *dict = dict_create(DICT_CAP);
    int *visited = calloc(DICT_CAP, sizeof(int));

    for (uint32_t i = 0; i < DICT_CAP; i++) {
        char key[16];
        sprintf(key, "key_%d", i+1);

        uint32_t cell = get_empty_cell(dict, key);
        int res = dict_put_int(dict, key, i);
        if(!res) return 1;

        //printf("key: %s, cell: %zu, state: %d, size: %d\n", key, cell, visited[cell], dict->size);
        assert(!visited[cell]);

        visited[cell] = 1;
    }

    dict_destroy(dict);
    return 1;
}

int full_dict_test(){
    Dict *dict = dict_create(DICT_CAP);
    int *visited = calloc(DICT_CAP, sizeof(int));

    for (uint32_t i = 0; i < DICT_CAP+1; i++) {
        char key[16];
        sprintf(key, "key_%d", i+1);

        uint32_t cell = get_empty_cell(dict, key);
        if(cell == INVALID_CELL && dict_last_error() != DICT_OK){
            //printf("%s\n", dict_error_string(dict_last_error()));
            return 1;
        }
        int res = dict_put_int(dict, key, i);

        //printf("key: %s, cell: %zu, state: %d, size: %d\n", key, cell, visited[cell], dict->size);
        assert(!visited[cell]);

        visited[cell] = 1;
    }

    dict_destroy(dict);
    return 0;
}

int put_full_test(){
  Dict *dict = dict_create(DICT_CAP);

  for (uint32_t i = 0; i < DICT_CAP+1; i++) {
        char key[16];
        sprintf(key, "key_%d", i+1);

        uint32_t cell = get_empty_cell(dict, key);
        if(cell == INVALID_CELL && dict_last_error() != DICT_OK){
            //printf("%s\n", dict_error_string(dict_last_error()));
            return 1;
        }
        int res = dict_put_int(dict, key, i);

        //printf("key: %s, cell: %zu, size: %d\n", key, cell, dict->size);
        assert(res);
    }
    
    assert(dict->capacity == DICT_CAP);
    dict_destroy(dict);
    return 0;
}

int take_all_test(){
  Dict *dict = dict_create(DICT_CAP);

  for (uint32_t i = 0; i < DICT_CAP+1; i++) {
    char key[16];
    sprintf(key, "key_%d", i+1);

    uint32_t cell = get_empty_cell(dict, key);
    if(cell == INVALID_CELL && dict_last_error() != DICT_OK){
        //printf("%s\n", dict_error_string(dict_last_error()));
        return 1;
    }
    int res = dict_put_int(dict, key, i);

    //printf("key: %s, cell: %zu, size: %d\n", key, cell, dict->size);
    assert(res);
  }
  assert(dict->capacity == DICT_CAP);

  for (uint32_t i = 0; i < DICT_CAP+1; i++) {
    char key[16];
    sprintf(key, "key_%d", i+1);

    DictValue v;
    int res = dict_take(dict, key, &v);
    if(!res){
      perror(dict_error_string(dict_last_error()));
    }

    //printf("key: %s, cell: %zu, size: %d\n", key, cell, dict->size);
    assert(res);
  }

  assert(is_empty(dict));
  assert(dict->capacity == 0);
  dict_destroy(dict);
  return 0;
}

int run_tests(){
  int res;

  res = collision_test();
  if(!res){
    perror("[!] Failed collision test.\n");
    return 0;
  } else {
    printf("[+] Success collion test.\n");
  }

  res = full_dict_test();
  if(!res){
    perror("[!] Failed full dictionary test.\n");
    return 0;
  } else {
    printf("[+] Success full dictionary test.\n");
  }
  
  res = put_full_test();
  if(!res){
    perror("[!] Failed put full test.\n");
    return 0;
  } else {
    printf("[!] Success put full dictionary test.\n");
  }

  res = take_all_test();
  if(!res){
    perror("[!] Failed take all test.\n");
    return 0;
  } else {
    printf("[!] Success take all dictionary test.\n");
  }

  return 1;
}