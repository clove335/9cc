#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "9cc.h"

void error(int i, char *s) {
  fprintf(stderr, "ERROR: expected %s, but got %s\n",
      s, tokens[i].input);
  exit(1);
}

bool consume(char *op) {
  if (strlen(op) != tokens[pos].len ||
      strncmp(tokens[pos].input, op, tokens[pos].len)) {
    return false;
  }
  pos++;
  return true;
}

void *tokenize(char *p) {
  int i = 0;
  while (*p) {
    // skip the spaces.
    if (isspace(*p)) {
      p++;
      continue;
    }
    
    if (strncmp(p, "==", 2) == 0) {
      tokens[i].ty = TK_EQ;
      tokens[i].input = "==";
      tokens[i].len = 2;
      i++;
      p += 2;
      continue;
    }  
    
    if (strncmp(p, "!=", 2) == 0) {
      tokens[i].ty = TK_NOT_EQ;
      tokens[i].input = p;
      tokens[i].len = 2;
      i++;
      p += 2;
      continue;
    }
    
    if (strncmp(p, ">=", 2) == 0) {
      tokens[i].ty = TK_L_EQ;
      tokens[i].input = p;
      tokens[i].len = 2;
      i++;
      p += 2;
      continue;
    }

    if ((strncmp(p, "<=", 2) == 0) ){
      tokens[i].ty = TK_L_EQ;
      tokens[i].input = p;
      tokens[i].len = 2;
      i++;
      p += 2;
      continue;
    }

    if ((strncmp(p, "<", 1) == 0) ){
      tokens[i].ty = TK_L_TH;
      tokens[i].input = p;
      tokens[i].len = 1;
      i++;
      p++;
      continue;
    }

    if ((strncmp(p, ">", 1) == 0) ){
      tokens[i].ty = TK_L_TH;
      tokens[i].input = p;
      tokens[i].len = 1;
      i++;
      p++;
      continue;
    }

    if (strncmp(p, "if", 2) == 0) {
      tokens[i].ty = TK_IF;
      tokens[i].input = "if";
      tokens[i].len = 2;
      i++;
      p += 2;
      continue;
    }
      
    if (strncmp(p, "else", 4) == 0) {
      tokens[i].ty = TK_ELSE;
      tokens[i].input = "else";
      tokens[i].len = 4;
      i++;
      p += 4;
      continue;
    }
      
    if (strncmp(p, "while", 5) == 0) {
      tokens[i].ty = TK_WHILE;
      tokens[i].input = "while";
      tokens[i].len = 5;
      i++;
      p += 5;
      continue;
    }
      
    if (strncmp(p, "for", 3) == 0) {
      tokens[i].ty = TK_FOR;
      tokens[i].input = "for";
      tokens[i].len = 3;
      i++;
      p += 3;
      continue;
    }
      
    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' 
        || *p == '(' || *p == ')' || *p == '=' || *p == ';') {
      tokens[i].ty = *p;
      tokens[i].input = p;
      tokens[i].len = 1;
      i++;
      p++;
      continue;
    }

    if (strncmp(p, "return", 6) == 0 && !isalnum(p[6])) {
      //tokens[i].ty = TK_RETURN;
      tokens[i].ty = TK_RETURN;
      tokens[i].input = "return";
      tokens[i].len = 6;
      i++;
      p += 6;
      continue;
    }
      
    if ('a' <= *p && *p <= 'z') {
      char save[256];
      int count = 0;

      tokens[i].ty = TK_IDENT;
      tokens[i].input = p;
      tokens[i].len = 1;
      
      do {
        save[count++] = *p;
        p++;
      } while (islower(*p));
      save[count] = '\0';

      char *copy = malloc(sizeof(char)*count);
      strcpy(copy, save);
      tokens[i].name = copy;
      i++;
      continue;
    }

    if (isdigit(*p)) {
      tokens[i].ty = TK_NUM;
      tokens[i].input = p;
      tokens[i].val = strtol(p, &p, 10);
      tokens[i].len = 1;
      i++;
      continue;
    }
    
    fprintf(stderr, "tokenize: error unexpected input. \n");
    exit(1);
  }
  
  tokens[i].ty = TK_EOF;
  tokens[i].input = p;
}

int expect(int line, int expected, int actual) {
  if (expected == actual) {
    if (tokens[pos].ty == '(' ||
        tokens[pos].ty == ';' ||
        tokens[pos].ty == ')') pos++;
    return 0;
  }
  fprintf(stderr, "%d: %d expected, but got %d \n",
      line, expected, actual);
  exit(1);
}

