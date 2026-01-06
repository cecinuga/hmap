#include <stdio.h>
#include <stdlib.h>
#include "dict.h"

int main(void){
  Dict *dict = dict_create(100);

  int prova = 4;

  int a = dict_put_int(dict, "a", 5);

  dict_cleanup(dict);
  dict_destroy(dict);

  return 0;
}