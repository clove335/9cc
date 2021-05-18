#include "9cc.h"
#include <stdlib.h>
#include <string.h>

Token tokens[1024];
Node *code[1024];
Map *func_symbols;
Map *symbols;

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
    map_put(env, node->name, (void *)(long)env->keys->len);
    return new_node('=', node, assign());
  }
  return node;
}

Node *expr() { return assign(); }

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
      error(pos, "')' for pair of '('");
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
      error(pos, "')' for pair of '('");
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

  if (tokens[pos].ty == ';' &&
      tokens[pos + 1].ty == TK_EOF)
  {
    error(++pos, "}");
  }
  if (!consume(";")) {
    error(pos, ";");
  }
  return node;
}

void program() {
  Node *node = malloc(sizeof(Node));
  node->definitions = new_vector();
  node->funcname = "";

  func_symbols = new_map();
  symbols = new_map();

  int i = 0;
  while (1) {
    if (tokens[pos].ty == '}' || tokens[pos].ty == TK_EOF)
      break;
    node = function_definition();
    vec_push(node->definitions, (void *) node);
    code[i] = node;
    i++;
  }
  code[i] = NULL;
}

// func-args = "(" (assign ("," assign)*)? ")"
static Node *func_args(void) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_FUNC_CALL;
  node->funcname = strndup(tokens[pos - 1].name, tokens[pos - 1].len);
  node->args_count = 0;
  if (consume("("))
    if (consume(")"))
      return node;

  node->args[0] = assign();
  node->args_count++;
  while (consume(",")) {
    if (node->args_count > 6) {
      error(pos, "too many arguments.");
    }
    node->args[node->args_count++] = assign();
  }
  expect(__LINE__, ')', tokens[pos].ty);
  return node;
}

Node *function_definition() {
  if (!consume("int")) {
    error(pos, "int");
  }
  if (tokens[pos].ty != TK_IDENT) {
    error(pos, "Function definition should begin with TK_IDENT.");
  }

  Symbol *func_pos = (Symbol *)new_symbol();
  map_put(func_symbols, tokens[pos].name, &func_pos->position);
  pos++;

  map_clear(symbols);
  int params_count = 0;
  consume("(");
  while (!consume(")")) {
    if (consume(",") || consume("int")) continue;
    if (params_count >= 6) {
      error(pos, "Up to 6 parameters");
    }
    if (tokens[pos].ty == TK_IDENT &&
        tokens[pos - 1].ty != TK_INT_DECL) {
      error(pos, "defined identifier");
    }

    Symbol *param = new_symbol();
    param->position = map_count(symbols) * 4 + 4;
    map_put(symbols, tokens[pos++].name, &param->position);
    params_count++;
  }

  Node *node = malloc(sizeof(Node));
  node->definitions = new_vector();
  node->ty = ND_FUNC_DEF;
  node->funcname = func_symbols->keys->data[func_symbols->count - 1];
  Node *comp_stmt = stmt();
  node->lhs = comp_stmt;
  node->vars_count = map_count(symbols);
  node->params_count = params_count;

  return node;
}

// term = "(" expr ")" | ident func-args? | num
Node *term() {
  Node *node = malloc(sizeof(Node));
  if (consume("(")) {
    node = expr();
    if (!consume(")")) {
      error(pos, "')' for pair of '('");
    }
    return node;
  }

  if (tokens[pos].ty == TK_IDENT) {
    if (tokens[pos + 1].ty == '(') {
      pos++;
      node = func_args();
      return node;
    }
    if (!map_get(symbols, tokens[pos].name)) error(pos, "defined identifier");
    node = new_node_ident(tokens[pos].name);
    if (map_get(func_symbols, tokens[pos].name)) {
      node->symbol = (Symbol *) map_get(func_symbols, tokens[pos].name);
    } else if (map_get(symbols, tokens[pos].name)) {
      node->symbol = (Symbol *) map_get(symbols, tokens[pos].name);
    } else {
      node->symbol = NULL;
    }
    pos++;
    return node;
  }

  if (tokens[pos].ty == TK_NUM) {
    return new_node_num(tokens[pos++].val);
  }

  if (tokens[pos].ty == TK_INT_DECL) {
    if (tokens[pos + 1].ty != TK_IDENT)
      error(pos + 1, "identifier after int declaration");
    if (tokens[pos + 2].ty != ';')
      error(pos + 2, "';' after int declaration and identifier");
    if (!map_get(symbols, tokens[pos + 1].name)) {
      Symbol *symbol = new_symbol();
      symbol->position = map_count(symbols) * 4 + 4;
      map_put(symbols, tokens[pos + 1].name, &symbol->position);
    }
    pos += 2;
    return node;
  }

}

Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume("*")) {
      node = new_node('*', node, unary());
    } else if (consume("/")) {
      node = new_node('/', node, unary());
    } else {
      return node;
    }
  }
}

Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume("+")) {
      node = new_node('+', node, mul());
    } else if (consume("-")) {
      node = new_node('-', node, mul());
    } else {
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
  if (consume("&")) {
    Node *node = malloc(sizeof(Node));
    node->ty = ND_ADDRESS;
    node->lhs = unary();
    return node;
  }
  if (consume("*")) {
    Node *node = malloc(sizeof(Node));
    node->ty = ND_DEREF;
    node->lhs = unary();
    return node;
  }
  return term();
}

Node *rel() {
  Node *node = add();

  for (;;) {
    if (consume("<=")) {
      node = new_node(ND_L_EQ, node, add());
    } else if (consume(">=")) {
      node = new_node(ND_L_EQ, add(), node);
    } else if (consume("<")) {
      node = new_node(ND_L_TH, node, add());
    } else if (consume(">")) {
      node = new_node(ND_L_TH, add(), node);
    } else {
      return node;
    }
  }
}

Node *equality() {
  Node *node = rel();
  for (;;) {
    if (consume("==")) {
      node = new_node(ND_EQ, node, rel());
    } else if (consume("!=")) {
      node = new_node(ND_NOT_EQ, node, rel());
    } else {
      return node;
    }
  }
}
