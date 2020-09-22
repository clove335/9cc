#include <string.h>
#include <stdlib.h>
#include "9cc.h"

Node *new_node(int ty, Node *lhs, Node *rhs) {
  Node *node = malloc(sizeof(Node));
  node->ty = ty;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_ident(char *name) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_IDENT;
  node->name = name;
  return node;
}

Node *new_node_num(int val) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_NUM;
  node->val = val;
  return node;
}

Node *assign() {
  Node *node = equality();
  if (consume("=")) {
    map_put(env, node->name, (void *) (long) env->keys->len);
    return new_node('=', node, assign());
  }
  return node;
}

Node *expr() {
  return assign();
}

Node *read_expr_stmt(void) {
  Node *node;
  node = malloc(sizeof(Node));
  node->ty = ND_EXPR_STMT;
  node->lhs = expr();
  return node;
}

Node *stmt() {
  Node *node;

  if (consume("return")) {
    node = malloc(sizeof(Node));
    node->ty = ND_RETURN;
    node->lhs = expr();
  } else {
    node = assign();
  }

  if (consume("if")) {
    node = malloc(sizeof(Node));
    node->ty = ND_IF;
    expect(__LINE__, '(', tokens[pos].ty);
    node->cond = expr();
    if (!consume(")")) {
      error(pos, "開きカッコ '(' に対応する閉じカッコ ')' がありません");
    }
    node->then = stmt();
    if (consume("else"))
      node->els = stmt();
    return node;
  }
  
  if (consume("while")) {
    node = malloc(sizeof(Node));
    node->ty = ND_WHILE;
    expect(__LINE__, '(', tokens[pos].ty);
    node->cond = expr();
    if (!consume(")")) {
      error(pos, "開きカッコ '(' に対応する閉じカッコ ')' がありません");
    }
    node->then = stmt();
    return node;
  }

  if (consume("for")) {
    node = malloc(sizeof(Node));
    node->ty = ND_FOR;
    expect(__LINE__, '(', tokens[pos].ty);
    if (!consume(";")) {
      node->init = read_expr_stmt();
      expect(__LINE__, ';', tokens[pos].ty);
    }
    if (!consume(";")) {
      node->cond = expr();
      expect(__LINE__, ';', tokens[pos].ty);
    }
    if (!consume(")")) {
      node->after = read_expr_stmt();
      expect(__LINE__, ')', tokens[pos].ty);
    }
    node->then = stmt();
    return node;
  }

  if (consume("{")) {
    Node *node = malloc(sizeof(Node));
    node->ty = ND_BLOCK;
    node->statements = new_vector();
    while (!consume("}")) {
      Node *stmt_in_block = stmt();
      vec_push(node->statements, (void *)stmt_in_block);
    }
    return node;
  }

  if (!consume(";")) {
    error(pos,";");
  }
  return node;
}

void program() {
  int i = 0;

  while (tokens[pos].ty != TK_EOF) {
    code[i++] = stmt();
  }
  code[i] = NULL;
}

// func-args = "(" (assign ("," assign)*)? ")"
static Node *func_args(void) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_FUNC_CALL;
  node->funcname = strndup(tokens[pos-2].name, tokens[pos-2].len);
  node->args_count = 0;
  if (consume(")"))
    return node;

  node->args[0] = assign();
  node->args_count++;
  while (consume(",")) {
    if (node->args_count >= 6) {
      error(pos, "too many arguments.");
    }
    node->args[node->args_count++] = assign();
  }
  expect(__LINE__, ')', tokens[pos].ty);
  return node;
}

// term = "(" expr ")" | ident func-args? | num
Node *term() {
  Node *node = malloc(sizeof(Node));
  if (consume("(")) {
    node = expr();
    if (!consume(")")) {
      error(pos, "開きカッコ '(' に対応する閉じカッコ ')' がありません");
    }
    return node;
  }
  
/*<<<<<<< HEAD:9cc.c*/
  if (tokens[pos].ty == TK_IDENT) {
    if (tokens[pos+1].ty == '(') {
      pos += 2;
      node = func_args();
      return node;
    }
    return new_node_ident(tokens[pos++].name);
  }

  if (tokens[pos].ty == TK_NUM) {
    return new_node_num(tokens[pos++].val);
  }

}

Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume("*")) {
      node = new_node('*', node, unary());
    }
    else if (consume("/")) {
      node = new_node('/', node, unary());
    }
    else {
      return node;
    }
  }
}

Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume("+")) {
      node = new_node('+', node, mul());
    }
    else if (consume("-")) {
      node = new_node('-', node, mul());
    }
    else {
      return node;
    }
  }
}

Node *unary() {
  if (consume("+")) {
    return term();
  }
  if (consume("-")) {
    return new_node('-', new_node_num(0), term());
  }
  return term();
}

Node *rel() {
  Node *node = add();

  for (;;) {
    if (consume("<=")) {
      node = new_node(ND_L_EQ, node, add());
    }
    else if (consume(">=")) {
      node = new_node(ND_L_EQ, add(), node);
    }
    else if (consume("<"))  {
      node = new_node(ND_L_TH, node, add());
    }
    else if (consume(">"))  {
      node = new_node(ND_L_TH, add(), node);
    }
    else {
      return node;
    }
  }
}

Node *equality() {
  Node *node = rel();
  for (;;) {
    if (consume("==")) {
      node = new_node(ND_EQ, node, rel());
    }
    else if (consume("!=")) {
      node = new_node(ND_NOT_EQ, node, rel());
    } 
    else { 
      return node;
    }
  }
}

