#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "9cc.h"

static int pos = 0;
Token tokens[100];
Map *env;

void error(int i, char *s) {
  fprintf(stderr, "ERROR: expected %s, but got %s\n",
      s, tokens[i].input);
  exit(1);
}

void *tokenize(char *p) {
  int i = 0;
  while (*p) {
    // skip the spaces.
    if (isspace(*p)) {
      p++;
      continue;
    }
    
    if (strncmp(p, "==", 2) == 0) {
      tokens[i].ty = TK_EQ;
      tokens[i].input = "==";
      tokens[i].len = 2;
      i++;
      p += 2;
      continue;
    }  
    
    if (strncmp(p, "!=", 2) == 0) {
      tokens[i].ty = TK_NOT_EQ;
      tokens[i].input = p;
      tokens[i].len = 2;
      i++;
      p += 2;
      continue;
    }
    
    if (strncmp(p, ">=", 2) == 0) {
      tokens[i].ty = TK_L_EQ;
      tokens[i].input = p;
      tokens[i].len = 2;
      i++;
      p += 2;
      continue;
    }

    if ((strncmp(p, "<=", 2) == 0) ){
      tokens[i].ty = TK_L_EQ;
      tokens[i].input = p;
      tokens[i].len = 2;
      i++;
      p += 2;
      continue;
    }

    if ((strncmp(p, "<", 1) == 0) ){
      tokens[i].ty = TK_L_TH;
      tokens[i].input = p;
      tokens[i].len = 1;
      i++;
      p++;
      continue;
    }

    if ((strncmp(p, ">", 1) == 0) ){
      tokens[i].ty = TK_L_TH;
      tokens[i].input = p;
      tokens[i].len = 1;
      i++;
      p++;
      continue;
    }

    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' 
        || *p == '(' || *p == ')' || *p == '=' || *p == ';') {
      tokens[i].ty = *p;
      tokens[i].input = p;
      tokens[i].len = 1;
      i++;
      p++;
      continue;
    }

    if (strncmp(p, "return", 6) == 0 && !isalnum(p[6])) {
      //tokens[i].ty = TK_RETURN;
      tokens[i].ty = TK_RETURN;
      tokens[i].input = "return";
      tokens[i].len = 6;
      i++;
      p += 6;
      continue;
    }
      
    if ('a' <= *p && *p <= 'z') {
      char save[256];
      int count = 0;

      tokens[i].ty = TK_IDENT;
      tokens[i].input = p;
      tokens[i].len = 1;
      
      do {
        save[count++] = *p;
        p++;
      } while (islower(*p));
      save[count] = '\0';

      char *copy = malloc(sizeof(char)*count);
      strcpy(copy, save);
      tokens[i].name = copy;
      i++;
      continue;
    }

    if (isdigit(*p)) {
      tokens[i].ty = TK_NUM;
      tokens[i].input = p;
      tokens[i].val = strtol(p, &p, 10);
      tokens[i].len = 1;
      i++;
      continue;
    }
    
    fprintf(stderr, "tokenize: error unexpected input. \n");
    exit(1);
  }
  
  tokens[i].ty = TK_EOF;
  tokens[i].input = p;
}

int expect(int line, int expected, int actual) {
  if (expected == actual) {
    return 0;
  }
  fprintf(stderr, "%d: %d expected, but got %d \n",
      line, expected, actual);
  exit(1);
}

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

Node *new_node(int ty, Node *lhs, Node *rhs) {
  Node *node = malloc(sizeof(Node));
  node->ty = ty;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_ident(char *name) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_IDENT;
  node->name = name;
  return node;
}

Node *new_node_num(int val) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_NUM;
  node->val = val;
  return node;
}

Node *code[1000];

Node *assign() {
  Node *node = equality();
  if (consume("=")) {
    map_put(env, node->name, (void *) (long) env->keys->len);
    return new_node('=', node, assign());
  }
  return node;
}

Node *expr() {
  return assign();
}

Node *stmt() {
  Node *node;

  //if (consume("return")) {
  if (consume("return")) {
    node = malloc(sizeof(Node));
    node->ty = ND_RETURN;
    node->lhs = assign();
  } else {
    node = assign();
  }
  
  if (!consume(";")) {
    error(pos,";");
  }

  return node;
}

void program() {
  int i = 0;

  while (tokens[pos].ty != TK_EOF) {
    code[i++] = stmt();
  }
  code[i] = NULL;
}

//int consume(int ty) {
bool consume(char *op) {
  //if (tokens[pos].ty != ty) {
  if (tokens[pos].ty == TK_RETURN) { pos++; return true; }
  //if (tokens[pos].input != *op ||
  if (strlen(op) != tokens[pos].len ||
      strncmp(tokens[pos].input, op, tokens[pos].len)) {
    return false;
  }
  pos++;
  return true;
}
void gen_lval(Node *node) {
  if (node->ty != ND_IDENT)
    error(pos, "代入の左辺値が変数ではありません");

  int count = (long)map_get(env, node->name);
  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", count*8);
  printf("  push rax\n");
}

void gen(Node *node) {
  if (node->ty == ND_RETURN) {
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return;
  }
  
  if (node->ty == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }

  if (node->ty == ND_IDENT) {
    gen_lval(node);
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    return;
  }

  if (node->ty == '=') {
    gen_lval(node->lhs);
    gen(node->rhs); 
    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov [rax], rdi\n");
    printf("  push rdi\n");
    return;
  }
  
  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->ty) {
  case '+':
    printf("  add rax, rdi\n");
    break;
  case '-':
    printf("  sub rax, rdi\n");
    break;
  case '*':
    printf("  mul rdi\n");
    break;
  case '/':
    printf("  mov rdx, 0\n");
    printf("  div rdi\n");
    break;
  case ND_EQ:
    printf("  cmp rax, rdi\n");
    printf("  sete al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_NOT_EQ:
    printf("  cmp rax, rdi\n");
    printf("  setne al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_L_EQ:
    printf("  cmp rax, rdi\n");
    printf("  setle al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_L_TH:
    printf("  cmp rax, rdi\n");
    printf("  setl al\n");
    printf("  movzb rax, al\n");
    break;
  }
  printf("  push rax\n");
}

Node *term() {
  if (consume("(")) {
    Node *node = assign();
    if (!consume(")")) {
      error(pos, "開きカッコ '(' に対応する閉じカッコ ')' がありません");
    }
    return node;
  }
  
/*<<<<<<< HEAD:9cc.c*/
  if (tokens[pos].ty == TK_IDENT) {
    return new_node_ident(tokens[pos++].name);
  }

  if (tokens[pos].ty == TK_NUM) {
    return new_node_num(tokens[pos++].val);
  }

}

Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume("*")) {
      node = new_node('*', node, unary());
    }
    else if (consume("/")) {
      node = new_node('/', node, unary());
    }
    else {
      return node;
    }
  }
}

Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume("+")) {
      node = new_node('+', node, mul());
    }
    else if (consume("-")) {
      node = new_node('-', node, mul());
    }
    else {
      return node;
    }
  }
}

Node *unary() {
  if (consume("+")) {
    return term();
  }
  if (consume("-")) {
    return new_node('-', new_node_num(0), term());
  }
  return term();
}

Node *rel() {
  Node *node = add();

  for (;;) {
    //Token *t = tokens;
    
    if (consume("<=")) {
      node = new_node(ND_L_EQ, node, add());
    }
    else if (consume(">=")) {
      node = new_node(ND_L_EQ, add(), node);
    }
    else if (consume("<"))  {
      node = new_node(ND_L_TH, node, add());
    }
    else if (consume(">"))  {
      node = new_node(ND_L_TH, add(), node);
    }
    else {
      return node;
    }
  }
}

Node *equality() {
  Node *node = rel();
  for (;;) {
    //Token *t = tokens;
    if (consume("==")) {
      node = new_node(ND_EQ, node, rel());
    }
    else if (consume("!=")) {
      node = new_node(ND_NOT_EQ, node, rel());
    } 
    else { 
      return node;
    }
  }
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
