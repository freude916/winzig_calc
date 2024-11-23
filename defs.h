#pragma once

#include <stdio.h>

#define INIT_TOKEN_COUNT 1024
#define MAX_TOKEN_LEN 256
#define STACK_SIZE 256
/// in fact, hash set leads its capacity down to 256
#define VAR_HASH_SIZE 65536
#define eps 1e-9

/// basic
enum Error {
    Success, // no error
    Running, // still running, don't know what will happen
    Error, // general error, not specified
    SyntaxError, // syntax error

    // following may happen in tokenizer
    InvalidChar, // invalid character

    // following may happen in parser
    UnexpectedToken, // unexpected token
    UnexpectedEnd, // unexpected end

    TooComplexGrammar, // expression or operator stack overflow

    // following may happen in interpreter runtime
    KeyboardInterrupt, // user interrupt

    RuntimeError, // runtime error
    MathError, // math error
};

# define report_error(key, error, msg) {\
    fprintf(stderr, "%s: %s\n", #error, msg); \
    key = error;\
    }

# define panic(msg, code) {\
    fprintf(stderr, "Panic: %s\n", msg); \
    exit(code);\
    }

enum DataTag {
    GNull, // nothing, only used in base, mark the end
    GNone, // nothing value, may appear in Expression?
    GError, // inside error happened

    GLiteral, GIdentifier,

    GExpr2, GExpr1, GExpression,

    GBuiltin, GPrint, GFunction,

    GStatement,

    GBlock,

    GWhile, GIf,
};

/// tokenizer
enum TokenType {
    TokenNull, // used for state mark, don't use in real token
    TokenWord, TokenNumber, TokenOperator, TokenLineSep,
};

struct Token {
    enum TokenType tag;
    char *token;
};

struct TokenData {
    struct Token *tokens; /// should end with GNull "\0", please push in the end.
    int count;
    int size; /// using in malloc or reallocate
    // int error;
    enum Error error; /// 0 for no err, 1 for grammar, 2 for invalid char, -1 for internal error
    int index; /// current index, for pop
};

struct TokenData *Ts_create();
void Ts_push(struct TokenData *tokens, enum DataTag tag, const char *token, unsigned long token_len);
void Ts_delete(struct TokenData *tokens);
void Ts_end(struct TokenData *tokens);
struct Token Ts_pop(struct TokenData *tokens);
struct Token Ts_peek(struct TokenData *tokens);
void Ts_advance(struct TokenData *tokens);

void tokenize(struct TokenData *tokens, const char *src);
/// end of tokenizer ------------------------------------------------------------------------------------
//

/// parser
//

/// function pointer for built-in functions
typedef long double (*d2dFunc)(struct Interpreter*, const long double); /// double d2d(double x);

/// float literal
struct Literal {
    long double value;
};

/// only variables now
struct Identifier {
    char *name;
};

/// Binary operation
/// Expr2 := Expression Op2 Expression.
/// left or right hand depends on operator? (maybe separate in future)
struct Expr2 {
    struct Expression *lhs;
    struct Expression *rhs;
    char *op;
    // +, -, *, /, &&, ||, ==, !=, <, <=, >, >=, etc.
    // =, +=, -=, *=, /=, %=, &=, |=, ^=, <<=, >>=, etc.
};

/// Unary operation (not implemented now)
/// Expr1 := Op1 Expression
struct Expr1 {
    struct Expression *expr;
    char *op;
    // possible unary operators: +, -, !, ~, etc.
};

///
/// Built-in math functions.
///
/// Builtin := func_name ( Expression )
///
/// now provided: abs, sin, cos, tan, asin, acos, atan, sqrt, log, log10, exp, ceil, floor, round, etc.
/// {@see {get_func}}
///
struct Builtin {
    d2dFunc func;
    char *name;
    struct Expression *expr;
};

///
/// Any Expression.
/// Expression := Literal | Identifier | Expr2 | Expr1 | Builtin
struct Expression {
    enum DataTag tag;

    union {
        struct Literal *literal;
        struct Identifier *identifier;
        struct Expr2 *expr2;
        struct Builtin *builtin;
        struct Expr1 *expr1;
    };
};

/// Too difficult to implement, just ignore it
// // struct Print {
// //     char *message;
// //     struct Expression expr;
// // };
//
// struct Function {
//     struct Identifier func;
//     struct Expression *expr; // ends with GNull
// };
//
// struct Return {
//     struct Expression *expr;
// };

/// Code block
struct Block {
    struct Statement **stmts; // ends with GNull
};

/// While loop
struct While {
    struct Expression *cond;
    struct Block *block;
};

/// If statement
struct If {
    struct Expression *cond;
    struct Block *then_block;
    struct Block *else_block; // may be GNull
};

/// Any statement
struct Statement {
    enum DataTag tag;

    union {
        struct Expression *expr;
        struct Builtin *builtin;
        // struct Print print;
        // struct Function function;
        // struct Return ret;
        struct Block *block;
        struct If *if_stmt;
        struct While *while_stmt;
    };
};

struct Parser {
    struct Block *result_block;
    enum Error error;
};

void Expr_delete(struct Expression *expr);
void Block_delete(struct Block *block);
void Statement_delete(struct Statement *stmt);

int operator_priority(const char* op);
void parse_file(struct Parser *parser, struct TokenData* tokens); // free tokens
struct Block* parse_block(struct Parser *parser, struct TokenData* tokens, int inner); // read from { to } or GNull
struct Statement* parse_statement(struct Parser *parser, struct TokenData* tokens); // read until TokenNewline
struct Expression* parse_expression(struct Parser *parser, struct TokenData* tokens, int inner); // read until TokenNewline
// struct Expression* parse_brace(struct TokenData* tokens); // read from ( to )

/// end of parser ------------------------------------------------------------------------------------
//

/// interpreter
struct Interpreter {
    long double variables[VAR_HASH_SIZE]; // hash set, except to save 256 elements
    enum Error error; // 0 for no err, 3 for runtime error
};

struct Interpreter* Interpreter_create() ;
long double Interpreter_get(struct Interpreter* interpreter, const char* name);
void Interpreter_set(struct Interpreter* interpreter, const char* name, const long double value);
long double interpret_Expression(struct Interpreter *interpreter, struct Expression *expr);
long double interpret_Block(struct Interpreter *interpreter, struct Block *block);
long double interpret_Statement(struct Interpreter *interpreter, struct Statement *stmt);
long double interpret_file(struct Interpreter *interpreter, struct Block *block);

///
void trace(){};