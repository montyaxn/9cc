#include "9cc.h"

#include <stdio.h>

//lvalのアドレスをスタックにプッシュする
void gen_lval(Node *node) {
  if (node->kind != ND_LVAR)
    error("代入の左辺値が変数ではありません");
  
  //raxにrbpを代入
  printf("  mov rax, rbp\n");
  //目的の変数が指し示すアドレスをraxに入れる
  printf("  sub rax, %d\n", node->offset);
  //そのアドレスをスタックに入れる
  printf("  push rax\n");
}

void gen(Node* node){
  switch(node->kind){
  case ND_NUM:
    printf("  push %d\n",node->val);
    return;
    // LVARの値をプッシュする
  case ND_LVAR:
    // LVARのアドレスをプッシュ
    gen_lval(node);
    // lVARのアドレスをraxに入れる
    printf("  pop rax\n");
    // LVARの中身をraxに入れる
    printf("  mov rax, [rax]\n");
    // LVARの中身をプッシュする
    printf("  push rax\n");
    return;

  case ND_ASSIGN:
    gen_lval(node->lhs);
    gen(node->rhs);
    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov [rax], rdi\n");
    printf("  push rdi\n");
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");
  switch (node->kind) {
  //四則演算
  case ND_ADD:
    printf("  add rax, rdi\n");
    break;

  case ND_SUB:
    printf("  sub rax, rdi\n");
    break;

  case ND_MUL:
    printf("  imul rax, rdi\n");
    break;

  case ND_DIV:
    printf("  cqo\n");
    printf("  idiv rdi\n");
    break;

  //比較
  case ND_EQ:
    printf("  cmp rax, rdi\n");
    printf("  sete al\n");
    printf("  movzb rax, al\n");
    break;

  case ND_NEQ:
    printf("  cmp rax, rdi\n");
    printf("  setne al\n");
    printf("  movzb rax, al\n");
    break;

  case ND_LS:
    printf("  cmp rax, rdi\n");
    printf("  setl al\n");
    printf("  movzb rax, al\n");
    break;

  case ND_LSEQ:
    printf("  cmp rax, rdi\n");
    printf("  setle al\n");
    printf("  movzb rax, al\n");
    break;

  

  }

  

  printf("  push rax\n");
}
