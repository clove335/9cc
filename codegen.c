#include "9cc.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

char args_reg[6][4] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
static int labelnum = 1;
static int continue_labelnum = 1;
static int break_labelnum = 1;

void gen_lval(Node *node) {
  if (node->ty != ND_IDENT && node->ty != ND_DEREF)
    error(pos, "identifier");

  int count = (long)node->symbol->position;
  if (node->symbol->value_type->type == INT) {
    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", count);
    printf("  push rax\n");
  }
  else if (node->symbol->value_type->type == POINTER) {
    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", count);
    printf("  push rax\n");
  }

}

void gen_func_def(Node *node) {
  if (strcmp(node->funcname, "main") == 0) {
    printf("  .global main\n");
  }

  printf("%s:\n", node->funcname);
  printf("  push rbp\n"); /* prologue. 26 Sizes of variable. */
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, %d\n", 26 * 8);

  for (int i = 0; i < node->params_count; i++) {
    printf("  mov rax, %s\n", args_reg[i]);
    printf("  mov QWORD PTR -%d[rbp], rax\n", i * 8 + 8);
  }

  gen(node);
  printf("  pop rax\n"); /* Load the value of all equatation to RAX */

  if (strcmp(node->funcname, "main") == 0) {
    printf(".L.return:\n"); /* and return it.                          */
  }
  printf("  mov rsp, rbp\n"); /* epilogue. */
  printf("  pop rbp\n");
  printf("  ret\n"); /* and return it.                          */
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
    if (!(*node).symbol)
      error(pos, "defined identifier");
    gen_lval(node);
    printf("  pop rax\n");
    printf("  mov rax, QWORD PTR [rax]\n");
    printf("  push rax\n");
    return;
  }

  if ((*node).ty == '=') {
    Symbol *symbol = node->lhs->symbol;
    gen_lval(node->lhs);

    if (symbol->value_type->type == INT) {
      gen(node->rhs);
      printf("  pop rdi\n");
      printf("  pop rax\n");
      printf("  mov QWORD PTR [rax], rdi\n");
      printf("  push rdi\n");
    } else if (symbol->value_type->type == POINTER) {
      gen(node->rhs);
      printf("  pop rdi\n");
      printf("  pop rax\n");
      printf("  mov QWORD PTR [rax], rdi\n");
      printf("  push rdi\n");
    }

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
    continue_labelnum = labelnum;
    break_labelnum = labelnum;
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

  if ((*node).ty == ND_DO_WHILE) {
    continue_labelnum = labelnum;
    break_labelnum = labelnum;
    int num = labelnum++;
    printf(".L.begin.%d:\n", num);
    gen(node->then);
    gen(node->cond);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je  .L.end.%d\n", num);
    printf("  jmp  .L.begin.%d\n", num);
    printf(".L.end.%d:\n", num);
    return;
  }

  if ((*node).ty == ND_FOR) {
    break_labelnum = labelnum;
    int num = labelnum++;
    continue_labelnum = labelnum;
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
    printf(".L.begin.%d:\n", continue_labelnum);
    if (node->after)
      gen(node->after);
    printf("  jmp .L.begin.%d\n", num);
    printf(".L.end.%d:\n", num);
    return;
  }

  if (node->ty == ND_CONTINUE) {
    printf("  jmp .L.begin.%d\n", continue_labelnum);
    return;
  }

  if (node->ty == ND_BREAK) {
    printf("  jmp .L.end.%d\n", break_labelnum);
    return;
  }

  if ((*node).ty == ND_BLOCK) {
    for (int i = 0; i < node->statements->len; i++) {
      gen((Node *)node->statements->data[i]);
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
    printf("  jmp .L.return\n"); /* and return it.                          */
    return;
  }

  if ((*node).ty == ND_ADDRESS) {
    gen_lval(node->lhs);
    return;
  }

  if (node->ty == ND_DEREF) {
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  mov rax, QWORD PTR [rax]\n");
    printf("  push rax\n");
    return;
  }

  if (node->ty == ND_SIZEOF) {
    Type *left_type = node->lhs->symbol->value_type;
    int size = 0;
    if (left_type->type == INT) size = 4;
    else if (left_type->type == POINTER) size = 8;
    printf("  push %d\n", size);
    return;
  }

  if (node->lhs != 0)
    gen(node->lhs);
  if (node->rhs != 0)
    gen(node->rhs);

  if (node->ty == '+') {
    Node *left = node->lhs;
    Node *right = node->rhs;
    Type *left_type = left->symbol->value_type;
    Type *right_type = right->symbol->value_type;
    if (left_type->type == INT && right_type->type == INT) {
      printf("  pop rdi\n");
      printf("  pop rax\n");
      printf("  add rax, rdi\n");
      printf("  push rax\n");
    }

    if (left_type->type == POINTER && right_type->type == INT) {
      int size = 0;
      if (left_type->pointer_to->type == INT) size = 4;
      else if (left_type->pointer_to->type == POINTER) size = 8;
      else error(pos, "the size of type (INT == 4 or POINTER == 8)");

      printf("  pop rax\n");
      printf("  mov rdx, %d\n", size);
      printf("  mul rdx\n");
      printf("  pop rcx\n");
      printf("  add rcx, rax\n");
      printf("  push rcx\n");
    }
  }

  if (node->ty == '-') {
    Node *left = node->lhs;
    Node *right = node->rhs;
    Type *left_type = left->symbol->value_type;
    Type *right_type = right->symbol->value_type;
    if (left_type->type == INT && right_type->type == INT) {
      printf("  pop rdi\n");
      printf("  pop rax\n");
      printf("  sub rax, rdi\n");
      printf("  push rax\n");
    }
    else if (left_type->type == POINTER && right_type->type == INT) {
      int size = 0;
      if (left_type->pointer_to->type == INT) size = 4;
      else if (left_type->pointer_to->type == POINTER) size = 8;
      else error(pos, "the size of type (INT == 4 or POINTER == 8)");

      printf("  pop rax\n");
      printf("  mov rdx, %d\n", size);
      printf("  mul rdx\n");
      printf("  pop rcx\n");
      printf("  sub rcx, rax\n");
      printf("  push rcx\n");
    }
  }

  if (node->ty == '*') {
    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mul rdi\n");
    printf("  push rax\n");
  }

  if (node->ty == '/') {
    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov rdx, 0\n");
    printf("  div rdi\n");
    printf("  push rax\n");
  }

  switch ((*node).ty) {
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
