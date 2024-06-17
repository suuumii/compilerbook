

// トークンの種類
typedef enum {
    TK_MARK, // 記号
    TK_IDENT, // 識別子
    TK_NUM, // 数値
    TK_RETURN, // return
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
extern Token *token;

extern char *user_input;

// 抽象構文木のノードの種類
typedef enum {
    ND_ADD, // +
    ND_SUB, // -
    ND_MUL, // *
    ND_DIV, // /
    ND_ASSIGN, // =
    ND_LVAR, // ローカル変数
    ND_EQ,  // ==
    ND_NE,  // !=
    ND_LT,  // <
    ND_LE,  // <=
    ND_NUM, // 整数
    ND_RETURN, // return
} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind kind;
    Node *next;    // Next node
    Node *lhs;
    Node *rhs;
    int val; // kindがND_NUMの場合のみ使う
    char name; // kindがND_LVARの場合のみ使う
    int offset; // kindがND_LVARの場合のみ使う
};

void error_at(char *loc, char *fmt, ...);
void error(char *fmt, ...);
Token *tokenize();
Node *program();
void gen(Node *node);
