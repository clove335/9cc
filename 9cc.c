#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* トークンの型を表す値 */
enum {
  TK_NUM = 256,
  TK_EOF,
};

enum {
  ND_NUM = 256,   /* Type of int Node  */
};

typedef struct Node {
  int ty;             /* Operator or ND_NUM */
  struct Node *lhs;   /* Left-hand side */
  struct Node *rhs;   /* Right-hand side */
  int val;            /* Use only when ty is ND_NUM */
} Node;

typedef struct {
  int ty;       //Type of Token
  int val;      //tyがTKNUMの場合、その数値
  char *input;  //STRING of Token
} Token;

Token tokens[100];

void error(int i, char *s) {
  fprintf(stderr, "ERROR: expected %s, but got %s\n",
      s, tokens[i].input);
  exit(1);
}

void tokenize(char *p) {
  int i = 0;
  while (*p) {
    // skip the spaces.
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')') {
      tokens[i].ty = *p;
      tokens[i].input = p;
      i++;
      p++;
      continue;
    }

    if (isdigit(*p)) {
      tokens[i].ty = TK_NUM;
      tokens[i].input = p;
      tokens[i].val = strtol(p, &p, 10);
      i++;
      continue;
    }
    
    fprintf(stderr, "tokenize: error unexpected input. \n");
    exit(1);
  }
  
  tokens[i].ty = TK_EOF;
  tokens[i].input = p;
}

Node *new_node(int ty, Node *lhs, Node *rhs) {
  Node *node = malloc(sizeof(Node));
  node->ty = ty;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_NUM;
  node->val = val;
  return node;
}

int consume(int ty, int *pos) {
  if (tokens[*pos].ty != ty) {
    return 0;
  }
  (*pos)++;
  return 1;
}

void gen(Node *node) {
  if (node->ty == ND_NUM) {
    printf("  push %d\n", node->val);
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
  }
  printf("  push rax\n");
}

Node *add(int *pos);

Node *term(int *pos) {
  if (consume('(', pos)) {
    Node *node = add(pos);
    if (!consume(')', pos)) {
      error(*pos, "開きカッコ '(' に対応する閉じカッコ ')' がありません");
    }
    return node;
  }

  if (tokens[*pos].ty == TK_NUM) {
    return new_node_num(tokens[(*pos)++].val);
  }
}

Node *mul(int *pos) {
  Node *node = term(pos);

  for (;;) {
    if (consume('*', pos)) {
      node = new_node('*', node, term(pos));
    }
    else if (consume('/', pos)) {
      node = new_node('/', node, term(pos));
    }
    else {
      return node;
    }
  }
}

Node *add(int *pos) {
  Node *node = mul(pos);

  for (;;) {
    if (consume('+', pos)) {
      node = new_node('+', node, mul(pos));
    }
    else if (consume('-', pos)) {
      node = new_node('-', node, mul(pos));
    }
    else {
      return node;
    }
  }
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }
  /* Tokenize and parse. */
  tokenize(argv[1]);
  
  int pos = 0;
  Node *node = add(&pos);
  printf(".intel_syntax noprefix\n"); /* Output the first half of Assembly. */
  printf(".global main\n");           /*                                    */
  printf("main:\n");                  /*                                    */
  
  gen(node);
  printf("  pop rax\n");  /* Load the value of all equatation to RAX */
  printf("  ret\n");      /* and return it.                          */
  return 0;
}
