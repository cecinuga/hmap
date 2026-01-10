#include <stdio.h>
#include <stdlib.h>
#include "dict.h"
#include "dict_err.h"

int main(void){
  Dict *dict = dict_create(100);

  int r = dict_put_string(dict, "a", "ciao");
  if(!r){
    int err = dict_last_error();
    perror(dict_error_string(err));
    return 1;
  }

  r = dict_upd_string(dict, "a", "ciaoooo");
  if(!r){
    int err = dict_last_error();
    perror(dict_error_string(err));
    return 1;
  }

  DictValue *val = malloc(sizeof(DictValue));
  r = dict_get(dict, "a", val);
  if(!r){
    int err = dict_last_error();
    perror(dict_error_string(err));
    return 1;
  }

  printf("%s \n", val->s);

  free(val->s);
  free(val);
  dict_destroy(dict);

  return 0;
}