#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "utils.h"

long string_to_ascii_long(const char *str){
    assert(strlen(str) <= MAX_KEY_LEN);

    size_t len = strlen(str);
    char *output = malloc(len * 3 + 1);
    
    if (!output) return 0;
    output[0] = '\0';   

    char temp[4];
    for(size_t i = 0; i < len; i++){
        snprintf(temp, sizeof(temp), "%d", (unsigned char)str[i]);
        strcat(output, temp);
    }
    
    char *endptr;
    long res = strtol(output, &endptr, 10);

    free(output);

    return res;
}