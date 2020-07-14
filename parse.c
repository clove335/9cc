#include <string.h>
#include <stdlib.h>
#include "9cc.h"

Map *env;

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

Node *code[1000];

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

Node *stmt() {
  Node *node;

  //if (consume("return")) {
  if (consume("return")) {
    node = malloc(sizeof(Node));
    node->ty = ND_RETURN;
    node->lhs = assign();
  } else {
    node = assign();
  }

  if (consume("if")) {
    node = malloc(sizeof(Node));
    node->ty = ND_IF;
    if (consume("(")) {
      node->cond = expr();
      if (!consume(")")) {
        error(pos, "開きカッコ '(' に対応する閉じカッコ ')' がありません");
      }
    }
    node->then = stmt();
    if (consume("else")) node->els = stmt();
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

Node *term() {
  if (consume("(")) {
    Node *node = assign();
    if (!consume(")")) {
      error(pos, "開きカッコ '(' に対応する閉じカッコ ')' がありません");
    }
    return node;
  }
  
/*<<<<<<< HEAD:9cc.c*/
  if (tokens[pos].ty == TK_IDENT) {
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
    //Token *t = tokens;
    
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
    //Token *t = tokens;
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

