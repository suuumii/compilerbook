#include "9cc.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>

// デバック用　トークンのstrを最後まで出力
void printToken(Token *tokenTmp) {
    for(;;) {
        printf("%.*s ", tokenTmp->len, tokenTmp->str);
        tokenTmp = tokenTmp->nextToken;
        if (tokenTmp->kind == TK_EOF) {
            printf("\n");
            break;
        }
    }
    exit(1);
}

// エラー出力するための関数
void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void error_at(char *loc, char *fmt, ...){
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, " "); //pos個の空白を出力
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

char *user_input;
Token *token;

int main(int argc, char **argv) {
    if(argc != 2){
        error("引数の個数が正しくありません");
        return 1;
    }

    user_input = argv[1];
    token = tokenize(user_input);
    // printToken(token);
    Node *node = program();

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // プロローグ　変数２６個の領域の確保
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, 208\n");

    // 先頭の指揮から順にコード生成
    for (Node *n = node; n; n = n->next) {
        gen(n);
        printf("  pop rax\n");
    }

    // エピローグ
    // 最後の式の結果がRAXに残っているのでそれが返り値になる
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return 0;
}
