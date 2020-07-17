#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "vector.h"

int pos = 0;
Token tokens[100];
Map *env;
Node *code[1000];

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
/*<<<<<<< HEAD:9cc.c*/

  printf(".intel_syntax noprefix\n"); /* Output the first half of Assembly. */
  printf(".global main\n");           /*                                    */
  printf("main:\n");                  /*                                    */
 
  printf("  push rbp\n");      /* prologue. 26 Sizes of variable. */
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, %d\n", env->keys->len * 8);

  for (int i = 0; code[i]; i++) {
    gen(code[i]);
    printf("  pop rax\n");  /* Load the value of all equatation to RAX */
  }
  printf("  mov rsp, rbp\n"); /* epilogue. */
  printf("  pop rbp\n");  
  printf("  ret\n");      /* and return it.                          */
  return 0;
}
