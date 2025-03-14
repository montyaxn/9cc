#include "9cc.h"

#include <stdio.h>


int main(int argc,char **argv){
  if (argc != 2) {
    fprintf(stderr,"引数の個数が正しくありません");
    return 1;
  }

  user_input = argv[1];

  tokenize(user_input);

  // printf("--- DUMP TOKEN ---\n");
  // dump_token(token);
  // printf("--- DUMP TOKEN END ---\n\n");

  program();

  // printf("--- DUMP NODES ---\n");
  // for(int i = 0; code[i];i++){
  //   printf("--- NODE [%d] ---\n",i);
  //   dump_node(code[i],0);
  //   printf("--- NODE [%d] END ---\n\n",i);
  // }
  // printf("--- DUMP NODES END ---\n\n");

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  //プロローグ
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, %d\n",locals ? locals->offset : 0);

  // 先頭の式から順にコード生成
  for (int i = 0; code[i]; i++) {
    gen(code[i]);

    // 式の評価結果としてスタックに一つの値が残っている
    // はずなので、スタックが溢れないようにポップしておく
    printf("  pop rax\n");
  }

  // リターンアドレスをrspに代入してret

  // rsp(スタックの先頭)はrbpのとこを指している
  printf("  mov rsp, rbp\n");
  // スタックの先頭はrbpの一つ前、すなわちリターンアドレスを指している
  // rbpの中身は前のrbpのアドレスを指す
  printf("  pop rbp\n");
  // 万全にリターン
  printf("  ret\n");
  return 0;
}
