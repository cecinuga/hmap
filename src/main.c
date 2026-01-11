#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "dict.h"
#include "dict_err.h"

int main(void){
    Dict *dict = dict_create(128);
    if (!dict) {
        fprintf(stderr, "%s\n", dict_error_string(dict_last_error()));
        return 1;
    }
    dict_put_int(dict, "age", 42);

    DictValue v;
    if (dict_get(dict, "age", &v)) {
        printf("%d\n", v.i);
    }

    return 0;
}
