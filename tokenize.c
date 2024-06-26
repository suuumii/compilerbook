#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>

#include "9cc.h"

Token *new_token(TokenKind kind, char *p, Token *cur, int len){
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = p;
    tok->len = len;
    cur->nextToken = tok; // 連結リストの、連結元のTokenのnextTokenに新規で作成したTokenのアドレスをセット
    return tok;
}

bool startSwitch(char *p, char *q){
    return memcmp(p, q, strlen(q)) == 0;
}

int is_alnum(char c) {
    return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z') || (c == '_');
}

Token *tokenize(char *p) {
    Token head;
    head.nextToken = NULL;

    // 連結元のTokenのアドレス
    // 新しくTokenを作成する際に、連結元のTokenのnextTokenにアドレスをセットするために、元のTokenのアドレスを格納して置く
    Token *cur = &head;

    while (*p) {
        // 空白文字ならスキップ
        if(isspace(*p)) {
            p++;
            continue;
        }

        if(strncmp(p, "return", 6) == 0 && !is_alnum(p[6])){
            cur = new_token(TK_RETURN, p, cur, 6);
            p += 6;
            continue;
        }

        if(startSwitch(p, "==") || startSwitch(p, "!=")
            || startSwitch(p, "<=") || startSwitch(p, ">=")) {
                cur = new_token(TK_MARK, p, cur, 2);
                p += 2;
                continue; 
        }

        if(strchr("+-*/()<>=;", *p)) {
            cur = new_token(TK_MARK, p, cur, 1);
            p++;
            continue;
        }

        if(is_alnum(*p)) {
            char *q = p;
            p++;

            while(is_alnum(*p) || ('0' <= *p && *p <= '9')){
                p++;
            }

            cur = new_token(TK_IDENT, q, cur, p-q);
            continue;
        }

        if(isdigit(*p)) {
            cur = new_token(TK_NUM, p, cur, 0);
            char *q = p;
            cur->val = strtol(p, &p, 10);
            cur->len = p - q;
            continue;
        }

        error_at(p, "トークナイズできません");
    }

    new_token(TK_EOF, p, cur, 0);

    return head.nextToken;
}
