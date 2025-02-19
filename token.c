#include "9cc.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *user_input;

Token *token;

void dump_token(Token *tok) {
  if (tok->kind == TK_EOF) {
    return;
  }

  switch (tok->kind) {
  case TK_EOF:
    return;

  case TK_IDENT:
    printf("TK_IDENT %c\n", tok->str[0]);
    break;

  case TK_NUM:
    printf("TK_NUM %d\n", tok->val);
    break;

  // case TK_RETURN:
  //   printf("TK_RETURN\n");
  //   break;

  // case TK_IF:
  //   printf("TK_IF\n");
  //   break;
  
  // case TK_ELSE:
  //   printf("TK_ELSE\n");

  case TK_RESERVED:
    printf("TK_RESERVED ");
    for (int i = 0; i < tok->len; i++) {
      printf("%c", tok->str[i]);
    }
    printf("\n");
    break;
  }
  dump_token(tok->next);
}

int is_alnum(char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') ||
         ('0' <= c && c <= '9') || (c == '_');
}

Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  // callocはメモリを0で埋める
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}

// tokenのイテレータのはじめを返す
void tokenize(char *p) {
  Token head;
  head.next = NULL;
  Token *cur = &head;
  

  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }
    if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
      cur = new_token(TK_RESERVED, cur, p, 6);
      p += 6;
      continue;
    }
    if (strncmp(p, "if", 2) == 0 && !is_alnum(p[2])) {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }
    if (strncmp(p, "else", 4) == 0 && !is_alnum(p[4])) {
      cur = new_token(TK_RESERVED, cur, p, 4);
      p += 4;
      continue;
    }
    if (strncmp(p, "==", 2) == 0 || strncmp(p, "<=", 2) == 0 ||
        strncmp(p, ">=", 2) == 0 || strncmp(p, "!=", 2) == 0) {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }
    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' ||
        *p == ')' || *p == '<' || *p == '>' || *p == '=' || *p == ';') {
      cur = new_token(TK_RESERVED, cur, p, 1);
      p++;
      continue;
    }
    if(isalpha(*p)){
      int len = 1;
      p++;
      while(is_alnum(*p)){
        len++;
        p++;
      }
      cur = new_token(TK_IDENT, cur, p-len, len);
      continue;
    }
    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 0);
      cur->val = strtol(p, &p, 10);
      continue;
    }
    error_at(p, "トークナイズできません");
  }

  new_token(TK_EOF, cur, p, 0);

  // iterの一番最初
  token = head.next;
}