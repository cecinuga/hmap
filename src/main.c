#include <stdio.h>
#include <stdlib.h>
#include "dict.h"

int main(void){
  Dict *dict = dict_create(100);

  int r = dict_put_string(dict, "b", "ciao");

  DictValue *str = malloc(sizeof(*str));
  str->s = malloc(100);

  dict_get(dict, "b", str);

  dict_destroy(dict);

  free(str->s);
  free(str);
  return 0;
}