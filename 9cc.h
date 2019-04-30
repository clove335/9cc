/* トークンの型を表す値 */
enum {
  TK_NUM = 256,
  TK_IDENT,
  TK_RETURN,
  TK_EOF
};

enum {
  ND_NUM = 256,   /* Type of int Node  */
  ND_IDENT,
  ND_RETURN
};

typedef struct Node {
  int ty;             /* Operator or ND_NUM */
  struct Node *lhs;   /* Left-hand side */
  struct Node *rhs;   /* Right-hand side */
  int val;            /* Use only when ty is ND_NUM */
  char *name;          /* Use only when ty is ND_IDENT */
} Node;

typedef struct {
  int ty;       //Type of Token
  int val;      //tyがTKNUMの場合、その数値
  char *name;   // if TK_IDENT -> name.
  char *input;  //STRING of Token
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

void error(int i, char *s);
void *tokenize(char *p);
int expect(int line, int expected, int actual); 
void runtest(); 
Node *new_node(int ty, Node *lhs, Node *rhs);
Node *new_node_num(int val);
/*<<<<<<< HEAD*/
Node *new_node_ident(char *name);
int consume(int ty);
void gen(Node *node);
void gen_lval(Node *node);
Node *add();
Node *term();
Node *mul();
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
/*>>>>>>> fa0fd4fd1b67a8c00c5534b101c8609ca324be23*/
