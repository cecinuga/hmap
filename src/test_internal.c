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

void run_test(){
    int res;
    res = collision_test();
    if(!res){
      perror("[!] Failed collision test.\n");
    } else {
      printf("[+] Success collion test.\n");
    }

    res = full_dict_test();
    if(!res){
      perror("[!] Failed full dictionary test.\n");
    } else {
      printf("[+] Success full dictionary test.\n");
    }
}