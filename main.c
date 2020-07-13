#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "9cc.h"

int pos = 0;
Token tokens[100];
Map *env;
Node *code[1000];

void runtest() {
  test_vector();
  test_map();
}

void test_vector() {
  Vector *vec = new_vector();
  expect(__LINE__, 0, vec->len);

  for (int i = 0; i < 100; i++) {
    vec_push(vec, (void *)(long)i);
  }

  expect(__LINE__, 100, vec->len);
  expect(__LINE__, 0, (long)vec->data[0]);
  expect(__LINE__, 50, (long)vec->data[50]);
  expect(__LINE__, 99, (long)vec->data[99]);

  printf("OK\n");
}

void test_map() {
  Map *map = new_map();
  expect(__LINE__, 0, (long)map_get(map, "foo"));

  map_put(map, "foo", (void *)2);
  expect(__LINE__, 2, (long)map_get(map, "foo"));
  
  map_put(map, "bar", (void *)4);
  expect(__LINE__, 4, (long)map_get(map, "bar"));

  map_put(map, "foo", (void *)6);
  expect(__LINE__, 6, (long)map_get(map, "foo"));
}

Vector *new_vector() {
  Vector *vec = malloc(sizeof(Vector));
  vec->data = malloc(sizeof(void *) * 16);
  vec->capacity = 16;
  vec->len = 0;
  return vec;
}

void vec_push(Vector *vec, void *elem) {
  if (vec->capacity == vec->len) {
    vec->capacity *= 2;
    vec->data = realloc(vec->data, sizeof(void *) * vec->capacity);
  }
  vec->data[vec->len++] = elem;
}

Map *new_map() {
  Map *map = malloc(sizeof(Map));
  map->keys = new_vector();
  map->vals = new_vector();
  return map;
}

void map_put(Map *map, char *key, int *val) {
  vec_push(map->keys, key);
  vec_push(map->vals, val);
}

void *map_get(Map *map, char *key) {
  for (int i = map->keys->len - 1; i >= 0; i--) {
    if (strcmp(map->keys->data[i], key) == 0) {
      return map->vals->data[i];
    }
  }
  return NULL;
}

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
