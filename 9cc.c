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
int pos;

void tokenize(char *p) {
  int i = 0;
  while (*p) {
    // skip the spaces.
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (*p == '+' || *p == '-') {
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
    
    fprintf(stderr, "トークナイズできません: %s\n", p);
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

int consume(int ty) {
  if (tokens[pos].ty != ty) {
    return 0;
  }
  pos++;
  return 1;
}

void gen(Node *node) {
  if (node->ty == ND_NUM) {
    printf("  push%d\n", node->val);
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

void error(int i) {
  fprintf(stderr, "予期しないトークンです: %s\n",
      tokens[i].input);
  exit(1);
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }
  /* Tokenize and parse. */
  tokenize(argv[1]);
  
  /* Output the first half of Assembly. */
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");


  /* Load the value of all equatation to RAX and return it. */
  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}
