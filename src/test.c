#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "dict.c"

int collision_test(){
    Dict *dict = dict_create(DICT_CAP);

    dict_destroy(dict);
    return 0;
}