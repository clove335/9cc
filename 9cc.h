#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

/* トークンの型を表す値 */
enum {
  TK_NUM = 256,
  TK_IDENT,
  TK_EQ,
  TK_NOT_EQ,
  TK_L_EQ,
  TK_L_TH,
  TK_RETURN,
  TK_IF,
  TK_ELSE,
  TK_WHILE,
  TK_FOR,
  TK_EOF
};

enum {
  ND_NUM = 256,   /* Type of int Node  */
  ND_IDENT,
  ND_EQ,
  ND_NOT_EQ,
  ND_L_EQ,
  ND_L_TH,
  ND_RETURN,
  ND_EXPR_STMT,
  ND_IF,
  ND_WHILE,	// "while"
  ND_FOR,	// "for"
  ND_BLOCK,	// { ... }
  ND_FUNC_CALL,
  ND_FUNC_DEF,
  ND_PROGRAM
};

typedef struct {
  void **data;
  int capacity;
  int len;
} Vector;

typedef struct {
  int count;
  Vector *keys;
  Vector *vals;
} Map;

typedef struct symbol {
  int position;
} Symbol;

typedef struct Node {
  int ty;             /* Operator or ND_NUM */
  struct Node *lhs;   /* Left-hand side */
  struct Node *rhs;   /* Right-hand side */

  struct Node *cond;  /* "if", "while" or "for" statement */
  struct Node *then;
  struct Node *els;
  struct Node *init;
  struct Node *after;

  Vector *statements; /* Block */
  Vector *definitions; /* Block */
  char *funcname;     /* Function call */
  struct Node *args[6];         /* arguments for Functions */
  int args_count;
  int vars_count;
  Symbol *symbol;

  int val;            /* Use only when ty is ND_NUM */
  char *name;         /* Use only when ty is ND_IDENT */
} Node;

typedef struct {
  int ty;       //Type of Token
  int val;      //tyがTKNUMの場合、その数値
  char *name;   // if TK_IDENT -> name.
  char *input;  //STRING of Token
  int len;
} Token;  

extern int pos;
extern Token tokens[1024];
extern Map *env;
extern Node *code[1024];
extern Map *func_symbols;
extern Map *symbols;

extern Vector *new_vector();
extern void vec_push(Vector *vec, void *elem);
extern void test_vector();
extern void test_map();
extern Map *new_map();
extern void map_put(Map *map, char *key, int *val);
extern void *map_get(Map *map, char *key); 
extern void *map_search(Map *map, char *key);
Symbol *new_symbol();
extern int map_count(Map *map);
extern void map_clear(Map *map);

void error(int i, char *s);
void *tokenize(char *p);
int expect(int line, int expected, int actual); 
void runtest(); 
Node *new_node(int ty, Node *lhs, Node *rhs);
Node *new_node_num(int val);
Node *new_node_ident(char *name);
bool consume(char *op);
void gen(Node *node);
void gen_lval(Node *node);
Node *add();
Node *term();
Node *mul();
Node *unary();
Node *assign();
Node *stmt();
void program();
void codegen();
Node *equality();
Node *expr();
Node *function_definition();

