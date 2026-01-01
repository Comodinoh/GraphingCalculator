#include <ctype.h>
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <clocale>
#include <cstdint>
#include <cmath>

#define VARIABLE_NAME "x"

typedef double (*func)(double);
typedef double (*op_func)(double, double);

double add(double x, double y) {
    return x + y;
}

double sub(double x, double y) {
    return x - y;
}

double mul(double x, double y) {
    return x * y;
}

double div(double x, double y) {
    return x / y;
}

double pow(double x, double y) {
    return pow(x, y);
}

double negate(double x, double y) {
    return -x;
}

enum class TokenKind {
    NUMBER,
    NAME,
    OP,
    PAREN_START,
    PAREN_END,
    COND,
    TKEOF,
};

enum class OperatorKind {
    PLUS = 0,
    MINUS,
    MUL,
    DIV,
    POW,
    UNARY_MINUS,
    COUNT,
};

enum class FunctionKind {
    SIN = 0,
    COS,
    TAN,
    ATAN,
    COTAN,
    ACOTAN,
    EXP,
    LN,
    ABS,
    SQRT,
    COUNT,
};

enum class ConstantsKind {
    PI = 0,
    COUNT
};

struct Operator {
    char         op;
    size_t       arity;
    bool         right_associative;
    op_func      f;
};

struct Constant {
    const char* name;
    double      value;
};

struct Function {
    const char* name;
    func f;
};

struct FunctionCall {
    Function f;
    double   arg;
};
struct OperatorCall {
    Operator op;
    double   x, y;
};

struct Token {
    TokenKind kind;
    char* name;
};

static Operator operator_table[] = {
    {'+', 2, false, add},
    {'-', 2, false, sub},
    {'-', 1, false, negate},
    {'*', 2, false, mul},
    {'/', 2, false, div},
    {'^', 2, true,  pow}
};

double cotan(double x) {
    return 1 / tan(x);
}

double acotan(double x) {
    return 1 / atan(x);
}

static Function functions_table[] = {
    {"sin", sin},
    {"cos", cos},
    {"tan", tan},
    {"atan", atan},
    {"cotan", cotan},
    {"acotan", acotan},
    {"exp", exp},
    {"ln", log},
    {"abs", abs},
    {"sqrt", sqrt},
};

static Constant constants_table[] = {
    {"pi", 3.14}
};


static Operator* getOperator(char op, size_t arity) {
    for (int i = 0; i < (size_t)OperatorKind::COUNT; i++) {
        if (operator_table[i].op == op && operator_table[i].arity == arity) {
            return &operator_table[i];
        }
    }

    return nullptr;
}

static bool isOperator(char op) {
    for (size_t i = 0; i < (size_t) OperatorKind::COUNT; i++) {
        if (operator_table[i].op == op) return true;
    }
    return false;
}

static Function* getFunction(const char* func) {
    for (size_t i = 0; i < (size_t)FunctionKind::COUNT; i++) {
        if (strcmp(functions_table[i].name, func) == 0) {
            return &functions_table[i];
        }
    }
    return nullptr;
}

static bool isFunctionValid(const char* func) {
    for (size_t i = 0; i < (size_t)FunctionKind::COUNT; i++) {
        if (strcmp(functions_table[i].name, func) == 0) {
            return true;
        }
    }
    return false;
}

static Constant* getConstant(const char* c) {
    for (size_t i = 0; i < (size_t)ConstantsKind::COUNT; i++) {
        if (strcmp(constants_table[i].name, c) == 0) {
            return &constants_table[i];
        }
    }
    return nullptr;
}

static bool isConstantValid(const char* c) {
    for (size_t i = 0; i < (size_t)ConstantsKind::COUNT; i++) {
        if (strcmp(constants_table[i].name, c) == 0) {
            return true;
        }
    }
    return false;
}

static const char* getTokenKindName(TokenKind kind) {
    switch (kind) {
    case TokenKind::NUMBER:
        return "Number";
    case TokenKind::NAME:
        return "Name";
    case TokenKind::OP:
        return "Operator";
    case TokenKind::COND:
        return "Conditional";
    case TokenKind::PAREN_START:
        return "(";
    case TokenKind::PAREN_END:
        return ")";
    default:
        return "Unknown";
    }
}

static Token EOF_TOKEN = { TokenKind::TKEOF, nullptr };

struct Tokenizer {
    size_t count = 0;
    size_t capacity = 4;
    Token* tokens = (Token*) malloc(sizeof(Token) * capacity);
    char* expr = nullptr;

    void add_token(Token* token) {
        if (count >= capacity) {
            capacity *= 2;
            tokens = (Token*)realloc(tokens, sizeof(Token) * capacity);
        }

        printf("Adding token %s -> %s\n", token->name, getTokenKindName(token->kind));
        
        tokens[count] = *token;
        count++;
    }

    void destroy() {
        for (size_t i = 0; i < count; i++) {
            free(tokens[i].name);
        }
        free(tokens);
        free(expr);
    }

    void print() {
        printf("Tokenized \" %s \":\n", expr);
        for (size_t i = 0; i < count; i++) {
            printf("  %s -> %s\n", tokens[i].name, getTokenKindName(tokens[i].kind));
        }
    }

    void tokenize(const char* expression) {
        assert(expression != nullptr && expression[0] != '\0');
        size_t len = strlen(expression);
        printf("Expression Length: %d\n", len);

        expr = (char*)malloc(sizeof(char) * (len + 1));
        strncpy(expr, expression, len + 1);
        printf("Tokenizing expression %s\n", expr);
        
        for (size_t i = 0; i < len; i++) {

            char c = expr[i];

            if (!isascii(c)) {
                printf("Input string is not ascii formatted");
                goto tokenizer_cleanup;
                return;
            }

            if (isspace(c)) {
                continue;
            }


            if (isdigit(c)) {
                size_t start = i;

                while (i < len && isdigit(expr[i])) i++;
                if (i < len && expr[i] == '.') {
                    i++;
                    while (i < len && isdigit(expr[i])) i++;
                }
                i--;
                // 2 3 4 5 
                // 5 - 2 = 3

                size_t n_len = i - start + 1;
                char* substr = (char*)malloc(sizeof(char) * (n_len + 1));
                memset(substr, 0, sizeof(char)*(n_len+1));
                memcpy(substr, &expr[start], sizeof(char) * (n_len));

                Token token = {
                    TokenKind::NUMBER,
                    substr
                };

                add_token(&token);
                continue;
            }

            if (isalpha(c)) {
                size_t start = i;
                while (i < len && isalnum(expr[i]) != 0) i++;
                i--;

                size_t n_len = i - start + 1;
                char* substr = (char*)malloc(sizeof(char) * (n_len + 1));
                memset(substr, 0, sizeof(char) * (n_len + 1));
                memcpy(substr, &expr[start], sizeof(char) * (n_len));

                Token token = {
                    TokenKind::NAME, substr
                };

                add_token(&token);
                continue;
            }
            if (c == '(') {
                char* str = (char*)malloc(sizeof(char)*2);
                memset(str, 0, sizeof(char) * 2);
                str[0] = c;

                Token token = {
                    TokenKind::PAREN_START,

                    str
                };

                add_token(&token);
                continue;
            }
            if (c == ')') {
                char* str = (char*)malloc(sizeof(char) * 2);
                memset(str, 0, sizeof(char) * 2);
                str[0] = c;

                Token token = {
                    TokenKind::PAREN_END,

                    str
                };

                add_token(&token);
                continue;
            }
            if (isOperator(c)) {
                char* str = (char*)malloc(sizeof(char)*2);
                memset(str, 0, sizeof(char)*2);
                str[0] = c;

                Token token = {
                    TokenKind::OP,
                    str
                };
                
                add_token(&token);
                continue;
            }


            fprintf(stderr, "Unexpected Token at position %uz of string \"%dds\"\n", i, expr);
            /*for (size_t i2 = 0; i2 < strlen("Unexpected Token at position %uz of string \"") + i - 1; i2++) {
                printf(" ");
            }
            printf("^\n");*/

        tokenizer_cleanup:


            destroy();
            exit(1);
        }
    }

};

struct ParseNode {
    enum class Kind {
        NUMBER, NAME, UNARY, BINARY, CALL
    };

    Kind kind;
    union {
        double n;
        Constant c;
        Operator op;
        Function call;
    };
};

struct ParseTree {
    Tokenizer* tokenizer;
    size_t token_index = 0;

    ParseNode* head = nullptr;

    void Init(Tokenizer* tok) {
        tokenizer = tok;
    }
    void Next() {
        token_index++;
    }

    bool atEnd() {
        return token_index >= tokenizer->count;
    }

    Token* peek() {
        if (token_index >= tokenizer->count) {
            fprintf(stderr, "Unexpected EOF");
            exit(1);
        }
        return &tokenizer->tokens[token_index];
    }

    Token* peekN(size_t n) {
        size_t pos = token_index + n;

        if (pos >= tokenizer->count) {
            fprintf(stderr, "Unexpected EOF");
            exit(1);
        }

        return &tokenizer->tokens[pos];
    }

    void expect(TokenKind kind, const char* what) {
        Token* token = peek();
        if (token->kind != kind) {
            fprintf(stderr, "Expected %s but found %s", what, token->name);
            exit(1);
        }
    }

    ParseNode* CreateParseNode(ParseNode::Kind kind) {
        ParseNode* node = (ParseNode*)malloc(sizeof(ParseNode));
        node->kind = kind;

        switch (kind) {
        case ParseNode::Kind::NUMBER:
        {
            node->n = strtod(peek()->name, nullptr);
            break;
        }
        case ParseNode::Kind::NAME:
        {
            node->c = *getConstant(peek()->name);
            break;
        }
        case ParseNode::Kind::BINARY:
        {
            node->op = *getOperator(peek()->name[0], 2);
            break;
        }
        case ParseNode::Kind::UNARY:
        {
            node->op = *getOperator(peek()->name[0], 1);
            break;
        }
        case ParseNode::Kind::CALL:
        {
            node->call = *getFunction(peek()->name);
            break;
        }
        }
        return node;
    }

    ParseNode* ParseToken() {
        Token* token = peek();
        switch (token->kind) {
        case TokenKind::NUMBER:
            return CreateParseNode(ParseNode::Kind::NUMBER);
        case TokenKind::OP:
        {
            return CreateParseNode(ParseNode::Kind::UNARY);
        }
        }
    }

    void ParseExpression(size_t min_prec) {

    }

    void Parse() {
        ParseExpression(1);
    }
};


int main() {
    Tokenizer tokenizer;

    tokenizer.tokenize("2.2 + sin(x, x)");
    tokenizer.print();
    return 0;
}