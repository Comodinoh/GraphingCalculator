#include <cctype>
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <clocale>
#include <cstdint>
#include <cmath>
#include <csignal>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>

using Vector2d = sf::Vector2<double>;

using namespace std;
const int LATIME_FEREASTRA = 800;
const int INALTIME_FEREASTRA = 600;

#define VARIABLE_NAME "x"

typedef double (*func)(double);
typedef double (*op_func)(double, double);

void strim(char *s)
{
    int i;

    while (isspace(*s)) s++;   // skip left side white spaces
    for (i = strlen(s) - 1; (isspace(s[i])); i--) ;   // skip right side white spaces
    s[i + 1] = '\0';
}

double dabs(double x) {
    return x > 0 ? x : -x;
}

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

//double pow(double x, double y) {
//    return pow(x, y);
//}

double cnegate(double x, double y) {
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
    E,
    PHI,
    COUNT
};

struct Operator {
    char         op;
    size_t       arity;
    size_t       precedence;
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

struct Token {
    TokenKind kind;
    size_t og_index;
    char* name;
};

static Operator operator_table[] = {
    {'+', 2, 1, false, add},
    {'-', 2, 1, false, sub},
    {'-', 1, 1, false, cnegate},
    {'*', 2, 2, false, mul},
    {'/', 2, 2, false, div},
    {'^', 2, 3, true,  pow}
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
    {"abs", dabs},
    {"sqrt", sqrt},
};

static Constant constants_table[] = {
    {"pi", 3.1415926},
    {"e", 2.71828182845904523536},
    {"phi", 1.6180339}
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

static bool isOperatorStr(const char* op) {
    for(size_t i = 0; i < (size_t) OperatorKind::COUNT; i++) {
        if(strcmp(&operator_table[i].op, op) == 0) return true;
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
        return "Open Paren";
    case TokenKind::PAREN_END:
        return "Close Paren";
    default:
        return "Unknown";
    }
}

static Token EOF_TOKEN = { TokenKind::TKEOF, 0, "EOF"};

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
        free(expr);
        count = 0;

        memset(tokens, 0, sizeof(Token)*capacity);
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
                    i,
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
                    TokenKind::NAME, i, substr
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
                    i,
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
                    i,
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
                    i,
                    str
                };
                
                add_token(&token);
                continue;
            }


            fprintf(stderr, "Unexpected Token at position %u of string \"%s\"\n", i-1, expression);
            /*for (size_t i2 = 0; i2 < strlen("Unexpected Token at position %uz of string \"") + i - 1; i2++) {
                printf(" ");
            }
            printf("^\n");*/

        tokenizer_cleanup:


            destroy();
            exit(1);
        }
    }

    void print_at_token(size_t index) {
        assert(index < count);

        Token* token = &tokens[index];

        size_t tk_len = strlen(token->name);

        printf("%s\n", expr);

        char* spacer = (char*)malloc(sizeof(char)*token->og_index+1);
        memset(spacer, ' ', sizeof(char)*token->og_index);
        spacer[token->og_index] = '\0';

        char* delim = (char*)malloc(sizeof(char)*tk_len+1);
        memset(delim, '~', sizeof(char)*tk_len);
        delim[tk_len] = '\0';

        printf("%s%s\n", spacer, delim);
        printf("%s^\n", spacer);

        free(spacer);
        free(delim);
    }
};

struct ParseNode {
    enum class Kind {
        NUMBER, CONSTANT, VARIABLE, UNARY, BINARY, CALL
    };

    Kind kind;
    ParseNode* r;
    ParseNode* l;
    union {
        double n;
        Constant c;
        Operator op;
        Function call;
    };
};

struct ParseTree {
    Tokenizer* tokenizer = nullptr;
    size_t token_index = 0;

    ParseNode* head = nullptr;

    void init(Tokenizer* tok) {
        tokenizer = tok;
        token_index = 0;
    }
    void next() {
        token_index++;
    }

    void nextN(size_t n) {
        token_index += n;
    }
    

    bool atEnd() {
        return token_index >= tokenizer->count;
    }

    bool atEndN(size_t n) {
        return token_index + n >= tokenizer->count;
    }
    

    Token* peek() {
        if (token_index >= tokenizer->count) {
            return &EOF_TOKEN;
        }
        return &tokenizer->tokens[token_index];
    }

    Token* peekN(size_t n) {
        size_t pos = token_index + n;

        if (pos >= tokenizer->count) {
            return &EOF_TOKEN;
        }

        return &tokenizer->tokens[pos];
    }

    void expect(TokenKind kind, const char* what) {
        Token* token = peek();
        if (token->kind != kind) {
            fprintf(stderr, "Expected token %s but found %s [index = %u]\n", what, peekN(-1)->name, token_index-1);
            tokenizer->print_at_token(token_index-1);
            exit(1);
        }
    }

    void expectNot(TokenKind kind) {
        Token* token = peek();
        if(token->kind == kind) {
            fprintf(stderr, "Unexpected token %s [index = %u]\n", token->name, token_index);
            tokenizer->print_at_token(token_index);
            exit(1);
        }
    }

    ParseNode* createParseNode(ParseNode::Kind kind) {
        ParseNode* node = (ParseNode*)malloc(sizeof(ParseNode));
        node->kind = kind;

        switch (kind) {
        case ParseNode::Kind::NUMBER:
        {
            node->n = strtod(peek()->name, nullptr);
            break;
        }
        case ParseNode::Kind::CONSTANT:
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

    ParseNode* parseToken() {
        Token* token = peek();
        switch (token->kind) {
        case TokenKind::NUMBER:
            return createParseNode(ParseNode::Kind::NUMBER);            
        case TokenKind::NAME:
            {
                if(peekN(1)->kind != TokenKind::PAREN_START) {
                    expectNot(TokenKind::PAREN_END);

                    ParseNode* node;
                    
                    if(strcmp(token->name, "x") == 0) {
                        node = createParseNode(ParseNode::Kind::VARIABLE);
                    }else {
                        if(!isConstantValid(token->name)) {
                            fprintf(stderr, "Unexpected symbol %s\n", token->name);
                            exit(1);
                        }
                        node = createParseNode(ParseNode::Kind::CONSTANT);
                    }

                    return node;
                }

                if(!isFunctionValid(token->name)) {
                    fprintf(stderr, "Invalid function name %s\n", token->name);
                    exit(1);
                }

                ParseNode* function = createParseNode(ParseNode::Kind::CALL);

                next();
                next();

                expectNot(TokenKind::PAREN_END);

                ParseNode* inside = parseExpression(1);

                next();
                expect(TokenKind::PAREN_END, ")");

                function->l = inside;
                
                return function;
            }
        case TokenKind::OP:
            {
                if(token_index == 0 || peekN(-1)->kind == TokenKind::PAREN_START) {
                    ParseNode* node = createParseNode(ParseNode::Kind::UNARY);
                    next();

                    ParseNode* inside = parseToken();
                    

                    node->r = inside;
                    
                    return node;
                }
                fprintf(stderr, "Unexpected OP %s at index %u\n", token->name, token_index);
                exit(1);
            }
        case TokenKind::PAREN_START:
            {
                next();
                expectNot(TokenKind::PAREN_END);
                ParseNode* inside = parseExpression(1);
                //printf("%d\n", peek()->kind);
                next();
                expect(TokenKind::PAREN_END, ")");

                return inside;
            }
        }
    }

    ParseNode* parseExpression(size_t min_prec) {
        if(atEnd()) {
            fprintf(stderr, "Unexpected EOF. Tried parsing Empty expression\n");
            tokenizer->print_at_token(token_index-1);
            exit(1);
        }
        
        ParseNode* left = parseToken();

        if(atEndN(1)) return left;

        Token* next = peekN(1);

        while(!atEnd() && isOperatorStr(next->name)  && getOperator(next->name[0], 2)->precedence >= min_prec) {
            Operator* op = getOperator(next->name[0], 2);
            size_t next_prec = op->right_associative ? op->precedence : op->precedence + 1;

            this->next();

            ParseNode* node = createParseNode(ParseNode::Kind::BINARY);


            if(atEndN(1)) {
                fprintf(stderr, "Unexpected EOF at token %s [index = %u]\n", peek()->name, token_index);
                tokenizer->print_at_token(token_index);
                exit(1);
            }

            this->next();
         
            ParseNode* right = parseExpression(next_prec);

            node->l = left;
            node->r = right;
            
            left = node;

            
            next = peekN(1);
        }

        return left;
    }
    

    double execute(double x, ParseNode* node) {
        switch(node->kind) {
        case ParseNode::Kind::NUMBER:
            {
                return node->n;
            }
        case ParseNode::Kind::BINARY:
            {
                double l = execute(x, node->l);
                double r = execute(x, node->r);

                return node->op.f(l, r);
            }
        case ParseNode::Kind::UNARY:
            {
                double n = execute(x, node->r);

                return node->op.f(n, 0);
            }
        case ParseNode::Kind::CALL:
            {
                double n = execute(x, node->l);

                return node->call.f(n);
            }
        case ParseNode::Kind::VARIABLE:
            {
                return x;
            }
        case ParseNode::Kind::CONSTANT:
            {
                return node->c.value;
            }
        }
    }

    void parse() {
        head = parseExpression(1);
    }
};

struct StringBuffer {
    char* buf = NULL;
    size_t len = 0;
    size_t capacity = 0;

    void append(const char* str) {
        size_t str_len = strlen(str);

        if(len + str_len >= capacity) {
            if(capacity != 0) {
                while(len + str_len >= capacity) {
                    capacity *= 2;
                }
            }else {
                capacity = str_len+1;
            }
            
            buf = (char*)realloc(buf, sizeof(char)*capacity);
            
        }
        strncpy(&buf[len], str, str_len+1);
        len += str_len;
    }

    void append_char(char c) {
        if(len >= capacity) {
            if(capacity != 0) {
                capacity *= 2;
            }else {
                capacity = 256;
            }
            
            buf = (char*)realloc(buf, sizeof(char)*capacity);
        }

        buf[len] = c;
        len++;
    }

    void append_char_and_check_null_term(char c) {
        append_char(c);
        check_null_term();
    }

    void remove_head() {
        if(len != 0) {
            buf[len-1] = '\0';
            len--;
        }
        
    }
    
    
    void check_null_term() {
        if(len != 0 && buf[len] != '\0') buf[len] = '\0';
    }
    
};


int main()
{

    Tokenizer tokenizer;
    ParseTree parser;

    sf::ContextSettings settings;

    settings.antiAliasingLevel = sf::RenderTexture::getMaximumAntiAliasingLevel();
    sf::RenderWindow fereastra(sf::VideoMode({LATIME_FEREASTRA, INALTIME_FEREASTRA}), "Sistem de Coordonate XOY", sf::Style::Default, sf::State::Windowed, settings);

    fereastra.setVerticalSyncEnabled(true);
    sf::View camera(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(20.0f, 15.0f));

    bool tragDeEcran = false;
    sf::Vector2i pozitieVecheMouse;

    StringBuffer buffer;
    sf::Font font;

    if(!font.openFromFile("MinecraftDefault-Regular.ttf")) {
        return 1;
    }
    
    sf::Text text(font);

    //sf::VertexBuffer buffer(sf::PrimitiveType::LineStrip, sf::VertexBuffer::Usage::Static);

    //size_t previousSize = 0;

    while (fereastra.isOpen())
    {
        while (std::optional event = fereastra.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                fereastra.close();


            if (sf::Event::MouseWheelScrolled* scroll = event->getIf<sf::Event::MouseWheelScrolled>())
            {
                if (scroll->delta > 0)
                {
                    camera.zoom(0.9f);
                }
                else
                {
                    camera.zoom(1.1f);
                }
            }


            if (sf::Event::MouseButtonPressed* pressed = event->getIf<sf::Event::MouseButtonPressed>())
            {
                tragDeEcran = true;
                pozitieVecheMouse = sf::Mouse::getPosition(fereastra);
            }
            if (event->is<sf::Event::MouseButtonReleased>())
            {
                tragDeEcran = false;
            }
            if (event->is<sf::Event::MouseMoved>() && tragDeEcran)
            {
                sf::Vector2i pozitieNouaMouse = sf::Mouse::getPosition(fereastra);

                sf::Vector2f delta = fereastra.mapPixelToCoords(pozitieVecheMouse, camera) - fereastra.mapPixelToCoords(pozitieNouaMouse, camera);
                
                camera.move(delta);
                                     
                pozitieVecheMouse = pozitieNouaMouse;
            }


            if (sf::Event::Resized* resized = event->getIf<sf::Event::Resized>())
            {
                float aspect = (float)resized->size.x / resized->size.y;
                camera.setSize({camera.getSize().y * aspect, camera.getSize().y});
            }

            if(sf::Event::TextEntered* text = event->getIf<sf::Event::TextEntered>()) {
                char c = (char)text->unicode;
                if(c == '\r') {
                    if(tokenizer.expr != nullptr) tokenizer.destroy();
                    tokenizer.tokenize(buffer.buf);
                    parser.init(&tokenizer);
                    parser.parse();
                } else if(c == '\b') {
                    buffer.remove_head();
                } else{
                    buffer.append_char(c);
                    printf("%c\n", c);
                }
            }
            
        }
        

        fereastra.clear(sf::Color(20, 20, 30));
        fereastra.setView(fereastra.getDefaultView());

        buffer.check_null_term();
        text.setString(buffer.buf);

        fereastra.draw(text);
        
        fereastra.setView(camera);

        sf::Vector2f centru = camera.getCenter();
        sf::Vector2f marime = camera.getSize();
        float stanga = centru.x - marime.x / 2;
        float dreapta = centru.x + marime.x / 2;
        float sus = centru.y - marime.y / 2;
        float jos = centru.y + marime.y / 2;

        sf::Vertex axaX[] =
        {
            sf::Vertex(sf::Vector2f(stanga, 0), sf::Color::White),
            sf::Vertex(sf::Vector2f(dreapta, 0), sf::Color::White)
        };
        sf::Vertex axaY[] =
        {
            sf::Vertex(sf::Vector2f(0, sus), sf::Color::White),
            sf::Vertex(sf::Vector2f(0, jos), sf::Color::White)
        };

        fereastra.draw(axaX, 2, sf::PrimitiveType::Lines);
        fereastra.draw(axaY, 2, sf::PrimitiveType::Lines);

        double distance = marime.x;

        //printf("Drawing at distance: %f\n", distance);

        if(parser.tokenizer != NULL) {

            size_t vertices = max((uint32_t)2, fereastra.getSize().x);
            sf::VertexArray array = sf::VertexArray{sf::PrimitiveType::LineStrip, vertices};
            //sf::Vertex* va = (sf::Vertex*)malloc(sizeof(sf::Vertex)*vertices);

            double dx = (marime.x) / (double)(vertices-1);

            for(size_t i = 0; i < vertices; i++) {
                double x = i*dx + stanga;
                array[i].position = sf::Vector2f(x, parser.execute(x, parser.head));
            }


            fereastra.draw(array);

        }

        
        fereastra.display();
    }
    return 0;
}
