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
    int len; // トークンの長さ
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

char *user_input;

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

// 数値の場合、Tokenを一つ読んでその数値を返す
int expect_number(){
    if(token->kind != TK_NUM){
        error_at(token->str, "数値ではありません");
    }
    int val = token->val;
    token = token->nextToken;
    return val;
}

// 次のTokenの期待が記号のためには、Tokenを一つ読みとって進める
void  expect(char *op){
    if(token->kind != TK_MARK || strlen(op) != token->len || memcmp(token->str, op, token->len)){
        error_at(token->str, "'%c'ではありません", op);
    }
    token = token->nextToken;
}

bool consume(char *op){
    if(token->kind != TK_MARK || strlen(op) != token->len || memcmp(token->str, op, token->len)){
        return false;
    }
    token = token->nextToken;
    return true;
}

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

Token *tokenize() {
    char *argv = user_input;
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

        if(startSwitch(argv, "==") || startSwitch(argv, "!=")
            || startSwitch(argv, "<=") || startSwitch(argv, ">=")) {
                cur = new_token(TK_MARK, argv, cur, 2);
                argv += 2;
                continue; 
        }

        if(strchr("+-*/()<>", *argv)) {
            cur = new_token(TK_MARK, argv, cur, 1);
            argv++;
            continue;
        }

        if(isdigit(*argv)) {
            cur = new_token(TK_NUM, argv, cur, 0);
            char *q = argv;
            cur->val = strtol(argv, &argv, 10);
            cur->len = argv - q;
            continue;
        }

        error_at(argv, "トークナイズできません");
    }

    new_token(TK_EOF, argv, cur, 0);

    return head.nextToken;
}

// 抽象構文木のノードの種類
typedef enum {
    ND_ADD, // +
    ND_SUB, // -
    ND_MUL, // *
    ND_DIV, // /
    ND_EQ,  // ==
    ND_NE,  // !=
    ND_LT,  // <
    ND_LE,  // <=
    ND_NUM, // 整数
} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind kind;
    Node *lhs;
    Node *rhs;
    int val; // kindがND_NUMの場合
};

Node *new_node(NodeKind kind, Node *lhs, Node *rhs){
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

Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

Node *expr() {
    Node *node = equality();

    return node; 
}

Node *equality() {
    Node *node = relational();

    for(;;) {
        if(consume("=="))
            node = new_node(ND_EQ, node, relational());
        else if(consume("!="))
            node = new_node(ND_NE, node, relational());
        else
            return node;
    }
}

Node *relational() {
    Node *node = add();

    for(;;){
        if(consume("<"))
            node = new_node(ND_LT, node, add());
        else if(consume("<="))
            node = new_node(ND_LE, node, add());
        else if(consume(">"))
            node = new_node(ND_LT, add(), node);
        else if(consume(">="))
            node = new_node(ND_LE, add(), node);
        else
            return node;
    }
}

Node *add() {
    Node *node = mul();

    for(;;) {
        if(consume("+"))
            node = new_node(ND_ADD, node, mul());
        else if(consume("-"))
            node = new_node(ND_SUB, node, mul());
        else
            return node;
    }
}

Node *mul() {
    Node *node = unary();

    for(;;) {
        if(consume("*"))
            node = new_node(ND_MUL, node, unary());
        else if(consume("/"))
            node = new_node(ND_DIV, node, unary());
        else
            return node;
    }
}

Node *unary() {
    if(consume("+"))
        return primary();
    if(consume("-"))
        return new_node(ND_SUB, new_node_num(0), primary());
    return primary();
}

Node *primary() {
    if(consume("(")){
        Node *node = expr();
        expect(")");
        return node;
    }

    return new_node_num(expect_number());
}

void gen(Node *node) {
    if(node->kind == ND_NUM) {
        printf("  push %d\n", node->val);
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");

    switch(node->kind) {
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
    case ND_EQ:  // ==
        printf("  cmp rax, rdi\n");
        printf("  sete al\n");
        printf("  movzb rax, al\n");
        break;
    case ND_NE:  // !=
        printf("  cmp rax, rdi\n");
        printf("  setne al\n");
        printf("  movzb rax, al\n");
        break;
    case ND_LT:  // <
        printf("  cmp rax, rdi\n");
        printf("  setl al\n");
        printf("  movzb rax, al\n");
        break;
    case ND_LE:  // <=
        printf("  cmp rax, rdi\n");
        printf("  setle al\n");
        printf("  movzb rax, al\n");
        break;
    }

    printf("  push rax\n");
}

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
}

int main(int argc, char **argv) {
    if(argc != 2){
        error("引数の個数が正しくありません");
        return 1;
    }

    user_input = argv[1];
    token = tokenize();
    // printToken(token);
    Node *node = expr();

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");
    
    // 抽象構文木を降りながらコード生成
    gen(node);

    // スタックのトップに敷き全体の値が残っているはずなので
    // それをRAXにロードして関数からの返り値とする
    printf("  pop rax\n");
    printf("  ret\n");
    return 0;
}
