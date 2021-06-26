#include "vector.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int pos = 0;
Map *env;

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }
  if (strcmp(argv[1], "-test") == 0) {
    runtest();
    return 0;
  }
  /* Tokenize and parse. */
  tokenize(argv[1]);
  env = new_map();
  program();
  codegen();

  return 0;
}
