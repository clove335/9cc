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
  char name;          /* Use only when ty is ND_IDENT */
} Node;

typedef struct {
  int ty;       //Type of Token
  int val;      //tyがTKNUMの場合、その数値
  char name;   // if TK_IDENT -> name.
  char *input;  //STRING of Token
} Token;  

typedef struct {
  void **data;
  int capacity;
  int len;
} Vector;

void error(int i, char *s);
void tokenize(char *p);
Vector *new_vector();
void vec_push(Vector *vec, void *elem);
int expect(int line, int expected, int actual); 
void runtest(); 
Node *new_node(int ty, Node *lhs, Node *rhs);
Node *new_node_num(int val);
Node *new_node_ident(char name);
int consume(int ty);
void gen(Node *node);
void gen_lval(Node *node);
Node *add();
Node *term();
Node *mul();
Node *assign();
Node *stmt();
void program();
