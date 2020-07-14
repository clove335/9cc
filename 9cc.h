#include <stdbool.h>
#include <stdio.h>

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
  ND_IF,
  ND_WHILE
};

typedef struct Node {
  int ty;             /* Operator or ND_NUM */
  struct Node *lhs;   /* Left-hand side */
  struct Node *rhs;   /* Right-hand side */

  struct Node *cond;         /* "if" or "while" statement */
  struct Node *then;
  struct Node *els;

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

typedef struct {
  void **data;
  int capacity;
  int len;
} Vector;

typedef struct {
  Vector *keys;
  Vector *vals;
} Map;

extern int pos;
extern Token tokens[100];

void error(int i, char *s);
void *tokenize(char *p);
int expect(int line, int expected, int actual); 
void runtest(); 
Node *new_node(int ty, Node *lhs, Node *rhs);
Node *new_node_num(int val);
/*<<<<<<< HEAD*/
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
Vector *new_vector();
void vec_push(Vector *vec, void *elem);
void test_vector();
void test_map();
Map *new_map();
void map_put(Map *map, char *key, int *val);
void *map_get(Map *map, char *key); 
Node *equality();
Node *expr();

/*>>>>>>> fa0fd4fd1b67a8c00c5534b101c8609ca324be23*/
