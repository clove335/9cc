#include <stdio.h>
#include <stdbool.h>
#include "9cc.h"

static int labelnum = 1;
Map *env;

void gen_lval(Node *node) {
  if (node->ty != ND_IDENT)
    error(pos, "代入の左辺値が変数ではありません");

  int count = (long)map_get(env, node->name);
  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", count*8);
  printf("  push rax\n");
}

void gen(Node *node) {
  if (node->ty == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }

  if (node->ty == ND_EXPR_STMT) {
    gen(node->lhs);
    printf("  add rsp, 8\n");
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

  if (node->ty == ND_IF) {
    int num = labelnum++;
    if (node->els) {
      gen(node->cond);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je  .L.else.%d\n", num);
      gen(node->then);
      printf("  jmp .L.end.%d\n", num);
      printf(".L.else.%d:\n", num);
      gen(node->els);
      printf(".L.end.%d:\n", num);
    } else {
      gen(node->cond);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je  .L.end.%d\n", num);
      gen(node->then);
      printf(".L.end.%d:\n", num);
    }
    return;
  }
  
  if (node->ty == ND_WHILE) {
    int num = labelnum++;
    printf(".L.begin.%d:\n", num);
    gen(node->cond);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je  .L.end.%d\n", num);
    gen(node->then);
    printf("  jmp .L.begin.%d\n", num);
    printf(".L.end.%d:\n", num);
    return;
  }

  if (node->ty == ND_FOR) {
    int num = labelnum++;
    if (node->init) 
      gen(node->init);
    printf(".L.begin.%d:\n", num);
    if (node->cond) {
      gen(node->cond);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je  .L.end.%d\n", num);
    }
    gen(node->then);
    if (node->after)
      gen(node->after);
    printf("  jmp .L.begin.%d\n", num);
    printf(".L.end.%d:\n", num);
    return;
  }

  if (node->ty == ND_BLOCK) {
    for (int i = 0; i < node->statements->len; i++) {
      gen((Node *) node->statements->data[i]);
    }
    return;
  }

  if (node->ty == ND_RETURN) {
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
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
