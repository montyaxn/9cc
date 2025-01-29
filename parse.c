#include "9cc.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// パーサののエラー処理
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}



void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, " "); // pos個の空白を出力
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

char *user_input;

Token *token;

void dump_token(Token* tok){
    if (tok->kind == TK_EOF){
        return;
    }
    switch(tok->kind){
    case TK_EOF:
        return;

    case TK_IDENT:
        printf("TK_IDENT %c\n",tok->str[0]);
        break;

    case TK_NUM:
        printf("TK_NUM %d\n",tok->val);
        break;

    case TK_RESERVED:
        printf("TK_RESERVED ");
        for(int i = 0; i<tok->len; i++){
            printf("%c",tok->str[i]);
        }
        printf("\n");
        break;
    }
    dump_token(tok->next);
}

Token* new_token(TokenKind kind, Token* cur, char *str, int len){
  //callocはメモリを0で埋める
  Token *tok = calloc(1,sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}

// tokenのイテレータのはじめを返す
void *tokenize(char* p){
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while(*p){
    if(isspace(*p)){
      p++;
      continue;
    }
    if(strncmp(p,"==",2) == 0||strncmp(p,"<=",2) == 0||strncmp(p,">=",2) == 0||strncmp(p,"!=",2) == 0){
      cur = new_token(TK_RESERVED,cur,p,2);
      p += 2;
      continue;
    }
    if(*p == '+' || *p == '-' || *p == '*' || *p== '/' || *p == '(' || *p == ')' || *p == '<' || *p == '>' || *p == '=' || *p == ';'){
      cur = new_token(TK_RESERVED,cur,p,1);
      p++;
      continue;
    }
    if ('a' <= *p && *p <= 'z') {
      cur = new_token(TK_IDENT, cur, p,1);
      p++;
      continue;
    }
    if(isdigit(*p)){
      cur = new_token(TK_NUM,cur,p,0);
      cur->val = strtol(p,&p,10);
      continue;
    }
    error_at(p,"トークナイズできません");
  }

  new_token(TK_EOF,cur,p,0);

  // iterの一番最初
  token = head.next;
}

// 以下はtokenに対する操作

// 次のトークンが期待している記号のときには、トークンを1つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
bool consume(char* op) {
  if (token->kind != TK_RESERVED ||
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    return false;
  token = token->next;
  return true;
}

Token* consume_ident(){
    if (token->kind != TK_IDENT){
        return NULL;
    }
    Token* tok = token;
    token = token->next;
    return tok;
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(char* op) {
  if (token->kind != TK_RESERVED || 
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    error_at(token->str,"'%c'ではありません", op);
  token = token->next;
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
int expect_number() {
  if (token->kind != TK_NUM){
    error_at(token->str,"数ではありません");
  }
  int val = token->val;
  token = token->next;
  return val;
}

// tokenがeofかの真偽
bool at_eof() {
  return token->kind == TK_EOF;
}


void dump_node(Node* node,int depth){
    if(node==NULL){
        return;
    }

    printf("%*s", depth * 4, "");
    depth++;

    switch(node->kind){
    case ND_NUM:
        printf("ND_NUM %d\n",node->val);
        break;
    case ND_ADD:
        printf("ND_ADD\n");
        break;
    case ND_SUB:
        printf("ND_SUB\n");
        break;
    case ND_MUL:
        printf("ND_MUL\n");
        break;
    case ND_DIV:
        printf("ND_DIV\n");
        break;
    case ND_EQ:
        printf("ND_EQ\n");
        break;
    case ND_NEQ:
        printf("ND_NEQ\n");
        break;
    case ND_LS:
        printf("ND_LS\n");
        break;
    case ND_LSEQ:
        printf("ND_LSEQ\n");
        break;
    case ND_ASSIGN:
        printf("ND_ASSIGN\n");
        break;
    case ND_LVAR:
        printf("ND_LVAR %d\n", node->offset);
        break;
    }

    dump_node(node->lhs,depth+1);
    dump_node(node->rhs,depth+1);
}

Node* new_node(NodeKind kind, Node* lhs, Node* rhs){
  Node* node = calloc(1,sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node* new_node_num(int val){
  Node* node = calloc(1,sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}



// tokenを解析してNode*を返す

// program    = stmt*
// stmt       = expr ";"
// expr       = assign
// assign     = equality ("=" assign)?
// equality   = relational ("==" relational | "!=" relational)*
// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
// add        = mul ("+" mul | "-" mul)*
// mul        = unary ("*" unary | "/" unary)*
// unary      = ("+" | "-")? primary
// primary    = num | ident | "(" expr ")"

void program();
Node* stmt();
Node* expr();
Node* assign();
Node* mul();
Node* unary();
Node* primary();
Node* equality();
Node* relational();
Node* add();

Node *code[100];

void program() {
  int i = 0;
  while (!at_eof())
    code[i++] = stmt();
  code[i] = NULL;
}

Node *stmt() {
  Node *node = expr();
  expect(";");
  return node;
}

Node *expr() {
  return assign();
}

Node *assign() {
  Node *node = equality();
  if (consume("="))
    node = new_node(ND_ASSIGN, node, assign());
  return node;
}


Node* equality(){
  Node* node = relational();

  for(;;){
    if(consume("==")){
      node = new_node(ND_EQ,node,relational());
    }else if(consume("!=")){
      node = new_node(ND_NEQ,node,relational());
    }else{
      return node;
    }
  }
}

Node* relational(){
  Node* node = add();
  for(;;){
    if(consume("<")){
      node = new_node(ND_LS,node,add());
    }else if(consume("<=")){
      node = new_node(ND_LSEQ,node,add());
    }else if(consume(">")){
      // node > add() は add() < node と同値
      node = new_node(ND_LS,add(),node);
    }else if(consume(">=")){
      // node >= add() は add() <= node と同値
      node = new_node(ND_LSEQ,add(),node);
    }else{
      return node;
    }
  }
}

Node* add(){
  Node* node = mul();

  for(;;){
    if(consume("+")){
      node = new_node(ND_ADD,node,mul());
    }else if(consume("-")){
      node = new_node(ND_SUB,node,mul());
    }else{
      return node;
    }
  }
}

Node* mul(){
  Node* node = unary();

  for(;;){
    if(consume("*")){
      node = new_node(ND_MUL,node,unary());
    }else if(consume("/")){
      node = new_node(ND_DIV,node,unary());
    }else{
      return node;
    }
  }
}

Node* unary(){
  if(consume("+")){
    return primary();
  }else if(consume("-")){
    return new_node(ND_SUB,new_node_num(0),primary());
  }else{
    return primary();
  }
}

Node* primary(){
  if(consume("(")){
    Node* node = expr();
    expect(")");
    return node;
  }

  Token* tok = consume_ident();
  if(tok){
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;
    node->offset = (tok->str[0] - 'a' + 1) * 8;
    return node;
  }

  return new_node_num(expect_number());
}