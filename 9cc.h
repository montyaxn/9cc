void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);


extern char* user_input;

// tokenの種類
typedef enum{
  TK_RESERVED,
  TK_IDENT,
  TK_NUM,
  TK_RETURN,
  TK_EOF
} TokenKind;

typedef struct Token Token;

// tokenのイテレータみたいなのを作る
struct Token {
  TokenKind kind; // トークンの型
  Token *next;    // 次の入力トークン
  int val;        // kindがTK_NUMの場合、その数値
  char *str;      // トークン文字列
  int len;        // kindがTK_RESERVEDの場合、そのトークンの長さ
};

extern Token* token;
void dump_token(Token* tok);

void tokenize(char* p);

typedef struct LVar LVar;

// ローカル変数の型
struct LVar {
  LVar *next; // 次の変数かNULL
  char *name; // 変数の名前
  int len;    // 名前の長さ
  int offset; // RBPからのオフセット
};

// ローカル変数
extern LVar *locals;

typedef enum {
  ND_NUM,

  //四則演算
  ND_ADD,
  ND_SUB,
  ND_MUL,
  ND_DIV,

  //比較演算子
  ND_EQ,
  ND_NEQ,
  ND_LS,
  ND_LSEQ,

  //代入
  ND_ASSIGN, 
  ND_LVAR,

  //RETURN
  ND_RETURN,
  
} NodeKind;

typedef struct Node Node;

struct Node{
  NodeKind kind;
  Node* lhs;
  Node* rhs;
  int val;
  int offset;
};

void dump_node(Node* node,int depth);

extern Node *code[100];
void program();

void gen(Node* node);