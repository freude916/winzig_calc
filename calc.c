#pragma once
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "defs.h"
// A Expression Calculator
// can eval +-*/(), math function call, variable, assignment, simple loop, if-else, function definition and call

/**
* Operators:
*
* calculator:   +, -, *, /, ==, !=, <, <=, >, >=
* assignment:   =, +=, -=, *=, /=
* logical:      &&, ||
* comparison:   ==, !=, <, <=, >, >=
* unary:        +, -, !, ~
*
* Known grammar:
* Define a function: fn <Function_Name>(<args...>){}; // TODO may not be implemented
*
* Known keywords:
* if, else, while, return
*/

enum TokenType {
    TokenNull, // used for state mark, don't use in real token
    TokenWord, TokenNumber, TokenOperator, TokenNewLine,
};

enum Tag {
    TNull, // nothing, only used in base
    TNone, // nothing value, may appear in Expression?

    TLiteral, TIdentifier,

    TExpr2, TExpr1, TExpression,

    TBuiltin, TPrint, TFunction,

    TStatement,

    TBlock,

    TWhile, TIf,
};

struct Literal {
    long double value;
};

struct Identifier {
    char *name;
};

struct Expr2 {
    struct Expression *lhs;
    struct Expression *rhs;
    char op;
    // +, -, *, /, &&, ||, ==, !=, <, <=, >, >=, etc.
    // =, +=, -=, *=, /=, %=, &=, |=, ^=, <<=, >>=, etc.
};

struct Expr1 {
    struct Expression *expr;
    char op;
    // possible unary operators: +, -, !, ~, etc.
};

struct Expression {
    enum Tag tag;

    union {
        struct Literal literal;
        struct Identifier identifier;
        struct Expr2 expr2;
        struct Expr1 expr1;
        struct None none;
    };
};

struct Builtin {
    double (*func)(double);

    struct Expression expr;
}; // abs, sin, cos, tan, asin, acos, atan, sqrt, log, log10, exp, ceil, floor, round, etc.

// struct Print {
//     char *message;
//     struct Expression expr;
// };
// TODO Too difficult to implement, just ignore it

struct Function {
    struct Identifier func;
    struct Expression *expr; // ends with TNull
};

struct Return {
    struct Expression expr;
};

struct Block {
    struct Statement *stmts; // ends with TNull
};

struct Statement {
    enum Tag tag;

    union {
        struct Expression expr;
        struct Builtin builtin;
        // struct Print print;
        struct Function function;
        struct Return ret;
        struct Block block;
    };
};

struct While {
    struct Expression cond;
    struct Block block;
};

struct If {
    struct Expression cond;
    struct Block then_block;
    struct Block else_block; // may be TNull
};

struct Token {
    enum TokenType tag;
    char *token;
};

struct TokenData {
    struct Token *tokens; // should end with TNull "\0", please push in the end.
    int count;
    int size; // using in malloc or reallocate
    int error; // 0 for no err, 1 for grammar, 2 for invalid char, -1 for internal error
    int index; // current index, pop
};


struct TokenData *Ts_create() {
    struct TokenData *tokens = malloc(sizeof(struct TokenData));
    tokens->tokens = malloc(sizeof(struct Token) * INIT_TOKEN_COUNT);
    tokens->count = 0;
    tokens->size = INIT_TOKEN_COUNT;
    tokens->error = 0;
    tokens->index = 0;
    return tokens;
}

void Ts_push(struct TokenData *tokens, enum Tag tag, const char *const token, int token_len) {
    if (!tokens) {
        return;
    }
    if (token_len <= 0) {
        token_len = strlen(token);
    }
    if (tokens->count >= tokens->size) {
        tokens->size *= 2;
        tokens->tokens = realloc(tokens->tokens, sizeof(struct Token) * tokens->size);
    }

    tokens->tokens[tokens->count].tag = tag;
    tokens->tokens[tokens->count].token = malloc((token_len + 1) * sizeof(char));
    strcpy(tokens->tokens[tokens->count].token, token);
    tokens->count++;
}

void Ts_end(struct TokenData *tokens) {
    Ts_push(tokens, TokenNull, "", 1);
}

void Ts_delete(struct TokenData *tokens) {
    free(tokens->tokens);
    free(tokens);
}

struct Token Ts_pop(struct TokenData *tokens) {
    if (tokens->index >= tokens->count) {
        tokens->error = -1; // internal error
        assert(0); // should not happen, we had push a TokenNull in the end before
    }
    return tokens->tokens[tokens->index++];
}

/**
 * Tokenize the source code
 *
 * @param src the source code, better with space between tokens, if not also OK
 * @return a list of tokens (ends with "\0")
 */
struct TokenData *tokenize(const char *src) {
    /// OK
    /// Passed test on 2024-11-06 22:07
    enum TokenType state = TokenNull;
    // Null for initial or blank, Word for word, Number for number, Operator for operator

    struct TokenData *tokens = Ts_create();
    char *token = malloc(sizeof(char) * MAX_TOKEN_LEN); // assume never exceed MAX_TOKEN_LEN. what hell can be so long?
    int token_len = 0;
#define PUSH_CHAR(c) token[token_len++] = c
#define PUSH_TOKEN(tag) \
    if (token_len > 0){\
        ;\
        PUSH_CHAR('\0');\
        Ts_push(tokens, tag, token, token_len);\
        token_len = 0;\
    }

    if (src == NULL || *src == '\0') {
        Ts_end(tokens);
        return tokens;
    }
    do {
        if (*src == ' ' || *src == '\t') {
            PUSH_TOKEN(state);
            state = TokenNull;
            continue;
        }
        if (*src == '\n' || *src == '\r') {
            PUSH_TOKEN(state);
            PUSH_TOKEN(TokenNewLine);
            while (*src == '\n' || *src == '\r') src++;
            state = TokenNull;
        }
        if (*src >= '0' && *src <= '9') {
            if (state == TokenOperator && token_len == 1 && (token[0] == '+' || token[0] == '-')) {
                PUSH_CHAR(*src);
                state = TokenNumber;
                continue;
            }
            if (state != TokenNull && state != TokenNumber) {
                PUSH_TOKEN(state);
            }
            PUSH_CHAR(*src);
            state = TokenNumber;
            continue;
        }
        if ((*src >= 'a' && *src <= 'z') || (*src >= 'A' && *src <= 'Z')) {
            if (state != TokenNull && state != TokenWord) {
                PUSH_TOKEN(state);
            }
            PUSH_CHAR(*src);
            state = TokenWord;
            continue;
        }
        /**
         * possible operator char
         * + - * / = > < & | !
         */
        if (*src == '+' || *src == '-' || *src == '*' || *src == '/' || *src == '^' ||
            *src == '>' || *src == '<' || *src == '=' ||
            *src == '&' || *src == '|' || *src == '!') {
            if (state != TokenNull && state != TokenOperator) {
                PUSH_TOKEN(state);
            }
            if (*src == '=') {
                // all operator has a version with '=' :)
                PUSH_CHAR(*src);
                PUSH_TOKEN(TokenOperator);
                state = TokenNull;
            } else {
                if (state == TokenOperator) {
                    tokens->error = 1; // grammar error. two operator in a row
                    break;
                }
                PUSH_CHAR(*src);
                state = TokenOperator;
            }
            continue;
        }
        if (*src == '(' || *src == ')' || *src == '{' || *src == '}') {
            if (state != TokenNull) {
                PUSH_TOKEN(state);
            }
            PUSH_CHAR(*src);
            PUSH_TOKEN(TokenOperator);
            state = TokenNull;
            continue;
        }
        tokens->error = 2; // invalid charactor
        break;
    } while (*++src != '\0');
    PUSH_TOKEN(state);
    Ts_end(tokens);
    return tokens;
#undef PUSH_CHAR
#undef PUSH_TOKEN
}

/**
 * Priority Table
 * Top
 * 1 ( )
 * 2 !
 * 3 ^ (pow)
 * 4 * /
 * 5 + -
 * 6 < <= > >=
 * 7 == !=
 * 8 & (logical and)
 * 9 | (logical or)
 * 10 = += -= *= /= ^= (calculate then assign)
 * 10 &= |= (logic then assign)
 */

int operator_priority(char *op) {
    if (*op == '\0') {
        return -1; // should not happen
    }
    if (op[1] == '=') {
        switch (*op) {
            case '+':
            case '-':
            case '*':
            case '/':
            case '^':
            case '&':
            case '|':
                return 10;
            case '=':
            case '!':
                return 7;
            case '<':
            case '>':
                return 6;
            default:
                return -1; // should not happen
        }
    }
    switch (*op) {
        case '(':
            case ')':
        return 1;
        case '!':
            return 2;
        case '^':
            return 3;
        case '*':
            case '/':
            return 4;
        case '+':
            case '-':
            return 5;
        case '<':
            case '>':
            return 6;
        // all 7 was processed above =
        case '&':
            return 8;
        case '|':
            return 9;
        default:
            return -1; // should not happen
    }
}

struct Expression* parse_expression(struct TokenData* tokens) {

}