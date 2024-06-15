#include "9cc.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>

// 次のTokenの期待が記号のためには、Tokenを一つ読みとって進める
void  expect(char *op){
    if(token->kind != TK_MARK || strlen(op) != token->len || memcmp(token->str, op, token->len)){
        error_at(token->str, "'%c'ではありません", op);
    }
    token = token->nextToken;
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

bool consume(char *op){
    if(token->kind != TK_MARK || strlen(op) != token->len || memcmp(token->str, op, token->len)){
        return false;
    }
    token = token->nextToken;
    return true;
}

// Consumes the current token if it is an identifier.
Token *consume_ident() {
  if (token->kind != TK_IDENT)
    return NULL;
  Token *t = token;
  token = token->nextToken;
  return t;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs){
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_lvar(char name) {
  Node *node = new_node(ND_LVAR, NULL, NULL);
  node->name = name;
  return node;
}

Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

bool at_eof(){
    return token->kind == TK_EOF;
}

Node *program(); // stmt*
Node *stmt(); // expr ";"
Node *expr(); // assign
Node *assign(); //equality ("=" assign)?
Node *equality(); // relational ("==" relational \ "!=" relational)*
Node *relational(); // add ("<" add | "<=" add | ">" add | ">=" add)*
Node *add(); // mul ("+" mul | "-" mul)*
Node *mul(); // unary ("*" unary | "/" unary)*
Node *unary(); // ("+" | "-")? primary
Node *primary(); // num | ident | "(" expr ")"

Node *code[100];

Node *program() {
  Node head;
  head.next = NULL;
  Node *cur = &head;

  while (!at_eof()) {
    cur->next = stmt();
    cur = cur->next;
  }
  return head.next;
}

Node *stmt() {
  Node *node = expr();
  expect(";");
  return node;
}

Node *expr() {
    Node *node = assign();

    return node;
}

Node *assign() {
    Node *node = equality();

    if(consume("="))
        node = new_node(ND_ASSIGN, node, assign());

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

    Token *tok = consume_ident();
    if (tok)
        return new_lvar(*tok->str);

    return new_node_num(expect_number());
}
