#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* トークンの型を表す値 */
enum {
  TK_NUM = 256,
  TK_EOF,
};

typedef struct {
  int ty;       //Type of Token
  int val;      //tyがTKNUMの場合、その数値
  char *input;  //STRING of Token
} Token;

Token tokens[100];

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

/* Function that outputs the error. */
void error(int i) {
  fprintf(stderr, "予期しないトークンです: %s\n",
          tokens[i].input);
  exit(1);
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません");
    return 1;
  }
  
  tokenize(argv[1]);
  
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");
  
  /* Check if the first input is digit to verify the equatation. */
  if (tokens[0].ty != TK_NUM) {
    error(0);
  }
  printf("  mov rax, %d\n", tokens[0].val);
  
  /* Output the assembly. */
  int i = 1;
  while (tokens[i].ty != TK_EOF) {
    if (tokens[i].ty == '+') {
      i++;
      if (tokens[i].ty != TK_NUM) {
        error(i);
      }
      printf("  add rax, %d\n", tokens[i].val);
      i++;
      continue;
    }

    if (tokens[i].ty == '-') {
      i++;
      if (tokens[i].ty != TK_NUM) {
        error(i);
      }
      printf("  sub rax, %d\n", tokens[i].val);
      i++;
      continue;
    }
    
    error(i);
  }

  printf("  ret\n");
  return 0;
}