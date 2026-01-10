#include <stdio.h>
#include <stdlib.h>
#include "dict.h"
#include "dict_err.h"

int main(void){
  Dict *dict = dict_create(100);

  int r = dict_put_string(dict, "b", "ciao");
  int r2 = dict_upd_string(dict, "b", "come");

  DictValue *str = malloc(sizeof(*str));

  dict_get(dict, "b", str);

  printf("%s\n", str->s);

  dict_destroy(dict);

  free(str->s);
  free(str);
  return 0;
}