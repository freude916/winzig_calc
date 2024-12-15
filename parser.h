# pragma once
# ifndef PARSER_H
# define PARSER_H
# include "base.h"


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
    long double (*func)(struct Interpreter *, long double);
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

struct Parser *Parser_create();

void Parser_refresh(struct Parser *parser);

void Parser_delete(struct Parser *parser);


void Expr_delete(struct Expression *expr);

void Block_delete(struct Block *block);

void Statement_delete(struct Statement *stmt);


void parse_file(struct Parser *parser, struct TokenData *tokens); // free tokens
struct Block *parse_block(struct Parser *parser, struct TokenData *tokens, int inner); // read from { to } or GNull
struct Statement *parse_statement(struct Parser *parser, struct TokenData *tokens); // read until TokenNewline
struct Expression *parse_expression(struct Parser *parser, struct TokenData *tokens, int inner);

// read until TokenNewline


void print_Statement(const struct Statement *statement);

void print_Block(const struct Block *block);

void print_Expression(const struct Expression *expression);

# endif //PARSER_H
