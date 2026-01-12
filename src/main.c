#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "test.h"
#include "dict_public.h"

int main(void){
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

    return 0;
}
