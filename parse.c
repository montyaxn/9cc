#include "9cc.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



// 以下はtokenに対する操作

// 次のトークンが期待している記号のときには、トークンを1つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
bool consume_reserved(char *op) {
  // TK_RESERVEDじゃない || lenがそろっていない ||
  // 文字列のメモリがlenの範囲で等しくない
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    return false;
  token = token->next;
  return true;
}

// bool consume_return() {
//   if (token->kind != TK_RETURN) {
//     return false;
//   }
//   Token *tok = token;
//   token = token->next;
//   return true;
// }

Token *consume_ident() {
  if (token->kind != TK_IDENT) {
    return NULL;
  }
  Token *tok = token;
  token = token->next;
  return tok;
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    error_at(token->str, "'%c'ではありません", op);
  token = token->next;
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
int expect_number() {
  if (token->kind != TK_NUM) {
    error_at(token->str, "数ではありません");
  }
  int val = token->val;
  token = token->next;
  return val;
}

// tokenがeofかの真偽
bool at_eof() { return token->kind == TK_EOF; }

LVar *locals;

LVar *find_lvar(Token *tok) {
  for (LVar *var = locals; var; var = var->next) {
    //! memcmpは結果がequalの時にtrue
    if (var->len == tok->len && !memcmp(var->name, tok->str, var->len))
      return var;
  }
  return NULL;
}

void dump_node(Node *node, int depth) {
  if (node == NULL) {
    return;
  }

  printf("%*s", depth * 4, "");
  depth++;

  switch (node->kind) {
  case ND_NUM:
    printf("ND_NUM %d\n", node->val);
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
  case ND_RETURN:
    printf("ND_RETURN\n");
    break;
  case ND_IF:
    printf("ND_IF\n");
    break;
  case ND_IF_ELSE:
    printf("ND_IF_ELSE\n");
    break;
  }

  dump_node(node->lhs, depth + 1);
  dump_node(node->rhs, depth + 1);
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

// tokenを解析してNode*を返す

// program    = stmt*
// stmt       = expr ";"
//            | "if" "(" expr ")" stmt ("else" stmt)?
//            | "while" "(" expr ")"
//            | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//            | "return" expr ";"
// expr       = assign
// assign     = equality ("=" assign)?
// equality   = relational ("==" relational | "!=" relational)*
// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
// add        = mul ("+" mul | "-" mul)*
// mul        = unary ("*" unary | "/" unary)*
// unary      = ("+" | "-")? primary
// primary    = num | ident | "(" expr ")"

void program();
Node *stmt();
Node *expr();
Node *assign();
Node *mul();
Node *unary();
Node *primary();
Node *equality();
Node *relational();
Node *add();

Node *code[100];

void program() {
  locals = calloc(1, sizeof(LVar));
  int i = 0;
  while (!at_eof())
    code[i++] = stmt();
  code[i] = NULL;
}

int if_id = 0;

Node *stmt() {
  Node *node;
  if (consume_reserved("return")) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_RETURN;
    node->lhs = expr();
    expect(";");
    return node;
  } else if (consume_reserved("if")){
    node = calloc(1, sizeof(Node));
    node->kind = ND_IF;
    node->val = if_id;
    if_id += 1;
    expect("(");
    node->lhs = expr();
    expect(")");
    node->rhs = stmt();
    if(consume_reserved("else")){
      Node *node_else = calloc(1,sizeof(Node));
      node_else->kind = ND_IF_ELSE;
      node_else->val = node->val;
      node_else->lhs = node;
      node_else->rhs = stmt();
      return node_else;
    }
    return node;
  } else {
    node = expr();
    expect(";");
    return node;
  }
}

Node *expr() { return assign(); }

Node *assign() {
  Node *node = equality();
  if (consume_reserved("="))
    node = new_node(ND_ASSIGN, node, assign());
  return node;
}

Node *equality() {
  Node *node = relational();

  for (;;) {
    if (consume_reserved("==")) {
      node = new_node(ND_EQ, node, relational());
    } else if (consume_reserved("!=")) {
      node = new_node(ND_NEQ, node, relational());
    } else {
      return node;
    }
  }
}

Node *relational() {
  Node *node = add();
  for (;;) {
    if (consume_reserved("<")) {
      node = new_node(ND_LS, node, add());
    } else if (consume_reserved("<=")) {
      node = new_node(ND_LSEQ, node, add());
    } else if (consume_reserved(">")) {
      // node > add() は add() < node と同値
      node = new_node(ND_LS, add(), node);
    } else if (consume_reserved(">=")) {
      // node >= add() は add() <= node と同値
      node = new_node(ND_LSEQ, add(), node);
    } else {
      return node;
    }
  }
}

Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume_reserved("+")) {
      node = new_node(ND_ADD, node, mul());
    } else if (consume_reserved("-")) {
      node = new_node(ND_SUB, node, mul());
    } else {
      return node;
    }
  }
}

Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume_reserved("*")) {
      node = new_node(ND_MUL, node, unary());
    } else if (consume_reserved("/")) {
      node = new_node(ND_DIV, node, unary());
    } else {
      return node;
    }
  }
}

Node *unary() {
  if (consume_reserved("+")) {
    return primary();
  } else if (consume_reserved("-")) {
    return new_node(ND_SUB, new_node_num(0), primary());
  } else {
    return primary();
  }
}

Node *primary() {
  if (consume_reserved("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  Token *tok = consume_ident();
  if (tok) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;

    LVar *lvar = find_lvar(tok);
    if (lvar) {

      node->offset = lvar->offset;
    } else {

      // 同じ名前の奴が存在しない場合
      // 新しくメモリ割り当て
      lvar = calloc(1, sizeof(LVar));
      // localsの一番左は一番新しい == 一番オフセットが深い
      lvar->offset = locals->offset + 8;

      node->offset = lvar->offset;

      // lvarにtokenの情報を入れる
      lvar->name = tok->str;
      lvar->len = tok->len;

      // lvarが今一番新しい
      lvar->next = locals;
      locals = lvar;
    }
    return node;
  }

  return new_node_num(expect_number());
}