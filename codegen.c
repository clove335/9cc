#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "9cc.h"

char args_reg[6][4] = { "rdi", "rsi", "rdx", "rcx", "r8", "r9" };
static int labelnum = 1;

void gen_lval(Node *node) {
  if (node->ty != ND_IDENT)
    error(pos, "identifier");

  int count = (long) node->symbol->position;
  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", count * 4);
  printf("  push rax\n");
}

void gen_func_def(Node *node) {
  if (strcmp(node->funcname, "main") == 0) {
    printf("  .global main\n");
  }

  printf("%s:\n", node->funcname);
  printf("  push rbp\n");      /* prologue. 26 Sizes of variable. */
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, %d\n", 26 * 8);

  for (int i = 0; i < node->params_count; i++) {
    printf("  mov rax, %s\n", args_reg[i]);
    printf("  mov [rbp-%d], rax\n", i * 16 + 16);
  }

  gen(node);
  printf("  pop rax\n");  /* Load the value of all equatation to RAX */

  if (strcmp(node->funcname, "main") == 0) {
    printf(".L.return:\n");      /* and return it.                          */
  }
  printf("  mov rsp, rbp\n"); /* epilogue. */
  printf("  pop rbp\n");
  printf("  ret\n");      /* and return it.                          */
}

void gen(Node *node) {
  if ((*node).ty == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }

  if ((*node).ty == ND_EXPR_STMT) {
    gen(node->lhs);
    printf("  add rsp, 8\n");
    return;
  }

  if ((*node).ty == ND_IDENT) {
    gen_lval(node);
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    return;
  }

  if ((*node).ty == '=') {
    gen_lval(node->lhs);
    gen(node->rhs); 
    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov [rax], rdi\n");
    printf("  push rdi\n");
    return;
  }

  if ((*node).ty == ND_IF) {
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
  
  if ((*node).ty == ND_WHILE) {
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

  if ((*node).ty == ND_FOR) {
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

  if ((*node).ty == ND_BLOCK) {
    for (int i = 0; i < node->statements->len; i++) {
      gen((Node *) node->statements->data[i]);
    }
    return;
  }

  if ((*node).ty == ND_FUNC_CALL) {
    int nargs = 0;
    for (; nargs < node->args_count; nargs++) {
      gen(node->args[nargs]);
    }

    for (int i = nargs - 1; i >= 0; i--) {
      if (i < node->args_count) {
        printf("  pop %s\n", args_reg[i]);
      }
    }

    printf("  call %s\n", node->funcname);
    printf("  push rax\n");
    return;
  }

  if ((*node).ty == ND_RETURN) {
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  push rax\n");
    printf("  jmp .L.return\n");      /* and return it.                          */
    return;
  }
  
  if (node->lhs != 0)
    gen(node->lhs);
  if (node->rhs != 0)
    gen(node->rhs);

  switch ((*node).ty) {
  case '+':
    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  add rax, rdi\n");
    printf("  push rax\n");
    break;
  case '-':
    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  sub rax, rdi\n");
    printf("  push rax\n");
    break;
  case '*':
    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mul rdi\n");
    printf("  push rax\n");
    break;
  case '/':
    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov rdx, 0\n");
    printf("  div rdi\n");
    printf("  push rax\n");
    break;
  case ND_EQ:
    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  cmp rax, rdi\n");
    printf("  sete al\n");
    printf("  movzb rax, al\n");
    printf("  push rax\n");
    break;
  case ND_NOT_EQ:
    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  cmp rax, rdi\n");
    printf("  setne al\n");
    printf("  movzb rax, al\n");
    printf("  push rax\n");
    break;
  case ND_L_EQ:
    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  cmp rax, rdi\n");
    printf("  setle al\n");
    printf("  movzb rax, al\n");
    printf("  push rax\n");
    break;
  case ND_L_TH:
    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  cmp rax, rdi\n");
    printf("  setl al\n");
    printf("  movzb rax, al\n");
    printf("  push rax\n");
    break;
  }
}

void codegen() {
  printf(".intel_syntax noprefix\n"); /* Output the first half of Assembly. */
  for (int i = 0; code[i]; i++) {
    gen_func_def(code[i]);
    printf("\n");
  }
}
