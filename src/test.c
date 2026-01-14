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
        if(!res) {
          printf("%s\n", dict_error_string(dict_last_error()));
          return 0;
        }

        //printf("key: %s, cell: %zu, state: %d, size: %d\n", key, cell, visited[cell], dict->size);
        assert(!visited[cell]);

        visited[cell] = 1;
    }
    dict_destroy(dict);
    
    return 1;
}

int succ_full_dict_test(){
  Dict *dict = dict_create(DICT_CAP);

  for (uint32_t i = 0; i < DICT_CAP; i++) {
      char key[16];
      sprintf(key, "key_%d", i+1);

      int res = dict_put_int(dict, key, i);
      if(!res) {
        printf("%s\n", dict_error_string(dict_last_error()));
        return 0;
      }
  }

  assert(dict->size == DICT_CAP);
  dict_destroy(dict);
  return 1;
}

int fail_full_dict_test(){
    Dict *dict = dict_create(DICT_CAP);

    for (uint32_t i = 0; i < DICT_CAP+1; i++) {
        char key[16];
        sprintf(key, "key_%d", i+1);
        int res = dict_put_int(dict, key, i);
        
        if(!res){
          if(i == DICT_CAP && dict_last_error() == DICT_ERR_DICT_FULL)
            return 1;

          printf("%s\n", dict_error_string(dict_last_error()));
          return 0;
        }
    }

    dict_destroy(dict);
    return 0;
}

int succ_put_full_test(){
  Dict *dict = dict_create(DICT_CAP);

  for (uint32_t i = 0; i < DICT_CAP; i++) {
        char key[16];
        sprintf(key, "key_%d", i+1);

        int res = dict_put_int(dict, key, i);
        if(!res) return 0;

        //printf("key: %s, cell: %zu, size: %d\n", key, cell, dict->size);
        assert(res);
    }

    assert(dict->capacity == DICT_CAP);
    dict_destroy(dict);
    return 1;
}

int fail_put_full_test(){
  Dict *dict = dict_create(DICT_CAP);

  for (uint32_t i = 0; i < DICT_CAP+1; i++) {
        char key[16];
        sprintf(key, "key_%d", i+1);

        int res = dict_put_int(dict, key, i);
        if(!res){
          if(i == DICT_CAP && dict_last_error() == DICT_ERR_DICT_FULL)
            return 1;

          printf("%s\n", dict_error_string(dict_last_error()));
          return 0;
        }

        //printf("key: %s, cell: %zu, size: %d\n", key, cell, dict->size);
        assert(res);
    }

    assert(dict->capacity == DICT_CAP);
    dict_destroy(dict);
    return 0;
}

int succ_take_all_test(){
  Dict *dict = dict_create(DICT_CAP);

  for (uint32_t i = 0; i < DICT_CAP; i++) {
    char key[16];
    sprintf(key, "key_%d", i+1);

    int res = dict_put_int(dict, key, i);
    if(!res) {
      printf("%s\n", dict_error_string(dict_last_error()));
      return 0;
    }

    assert(res);
  }
  assert(dict->capacity == DICT_CAP);
  for (uint32_t i = 0; i < DICT_CAP; i++) {
    char key[16];
    sprintf(key, "key_%d", i+1);

    DictValue v;
    int res = dict_take(dict, key, &v);
    
    if(!res){
      printf("%s: %s\n", dict_error_string(dict_last_error()), key);
      return 0;
    }

    //printf("key: %s, cell: %zu, size: %d\n", key, cell, dict->size);
    assert(res);
  }
  assert(is_empty(dict));
  assert(dict->size == 0);
  dict_destroy(dict);
  return 1;
}

#define FAIL_TAKE_ALL_TEST_UPPRBND 120
#define FAIL_TAKE_ALL_TEST_LWRBND 20
int fail_take_all_test(){ //STILL IN WIP
  Dict *dict = dict_create(DICT_CAP);

  for (uint32_t i = FAIL_TAKE_ALL_TEST_LWRBND; i < FAIL_TAKE_ALL_TEST_UPPRBND; i++) {
    char key[16];
    sprintf(key, "key_%d", i+1);

    int res = dict_put_int(dict, key, i);
    if(!res) {
      printf("%s\n", dict_error_string(dict_last_error()));
      return 0;
    }

    assert(res);
  }
  assert(dict->size == 100);
  for (uint32_t i = FAIL_TAKE_ALL_TEST_UPPRBND; i > FAIL_TAKE_ALL_TEST_LWRBND-10; i--) {
    char key[16];
    sprintf(key, "key_%d", i);

    DictValue v;
    int res = dict_take(dict, key, &v);
    if(!res){
      if(dict_last_error() == DICT_ERR_NOT_FOUND && i == 20)
        return 1;

      printf("%s\n", dict_error_string(dict_last_error()));
      return 0;
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
    perror("[!] Failed collision test.");
    return 0;
  } else {
    printf("[+] Success collion test.\n");
  }

  res = succ_full_dict_test();
  if(!res){
    perror("[!] Failed succ_full_dict_test.");
    return 0;
  } else {
    printf("[+] Success succ_full_dict_test.\n");
  }
  res = fail_full_dict_test();
  if(!res){
    perror("[!] Failed fail_full_dict_test.");
    return 0;
  } else {
    printf("[+] Success fail_full_dict_test.\n");
  }
  
  res = succ_put_full_test();
  if(!res){
    perror("[!] Failed succ_put_full_test.");
    return 0;
  } else {
    printf("[+] Success succ_put_full_test.\n");
  }
  
  res = fail_put_full_test();
  if(!res){
    perror("[!] Failed fail_put_full_test.");
    return 0;
  } else {
    printf("[+] Success fail_put_full_test.\n");
  }

  res = succ_take_all_test();
  if(!res){
    perror("[!] Failed succ_take_all_test.");
    return 0;
  } else {
    printf("[+] Success succ_take_all_test.\n");
  }

  res = fail_take_all_test();
  if(!res){
    perror("[!] Failed fail_take_all_test.");
    return 0;
  } else {
    printf("[+] Success fail_take_all_test.\n");
  }

  return 1;
}