#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>

// トークンの種類
typedef enum {
    TK_MARK, // 記号
    TK_NUM, // 数値
    TK_EOF, // 入力の終わりを示すトークン
} TokenKind;

typedef struct Token Token;

struct Token {
    TokenKind kind;  // トークンの種類
    int val;  // トークンタイプがTK_NUMの場合の、数値
    char *str; // トークンの文字列
    Token *nextToken;
};

// 現在処理しているトークン
Token *token;

// エラー出力するための関数
void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// 数値の場合、Tokenを一つ読んでその数値を返す
int expect_number(){
    if(token->kind != TK_NUM){
        error("数値ではありません");
    }
    int val = token->val;
    token = token->nextToken;
    return val;
}

// 次のTokenの期待が記号のためには、Tokenを一つ読みとって進める
void  expect(char op){
    if(token->kind != TK_MARK || token->str[0] != op){
        error("'%c'ではありません", op);
    }
    token = token->nextToken;
}

bool consume(char op){
    if(token->kind != TK_MARK || token->str[0] != op){
        return false;
    }
    token = token->nextToken;
    return true;
}

Token *new_token(TokenKind kind, char *p, Token *cur){
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = p;
    cur->nextToken = tok; // 連結リストの、連結元のTokenのnextTokenに新規で作成したTokenのアドレスをセット
    return tok;
}

Token *tokenize(char *argv) {
    Token head;
    head.nextToken = NULL;

    // 連結元のTokenのアドレス
    // 新しくTokenを作成する際に、連結元のTokenのnextTokenにアドレスをセットするために、元のTokenのアドレスを格納して置く
    Token *cur = &head;

    while (*argv) {
        // 空白文字ならスキップ
        if(isspace(*argv)) {
            argv++;
            continue;
        }

        if(*argv == '+' || *argv == '-') {
            cur = new_token(TK_MARK, argv, cur);
            argv++;
            continue;
        }

        if(isdigit(*argv)) {
            cur = new_token(TK_NUM, argv, cur);
            cur->val = strtol(argv, &argv, 10);
            continue;
        }

        error("トークナイズできません");
    }

    new_token(TK_EOF, argv, cur);

    return head.nextToken;
}

int main(int argc, char **argv) {
    if(argc != 2){
        fprintf(stderr, "引数の個数が正しくありません。\n");
        return 1;
    }

    token = tokenize(argv[1]);

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");
    printf("  mov rax, %d\n", expect_number());

    while(token->kind != TK_EOF){
        if(consume('+')){
            printf("  add rax, %d\n", expect_number());
            continue;
        }

        expect('-');
        printf("  sub rax, %d\n", expect_number());
    }

    printf("  ret\n");
    return 0;
}
