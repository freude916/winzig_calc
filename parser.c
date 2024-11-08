# include <stdlib.h>
# include <string.h>
# include <math.h>
# include "defs.h"
# include "base.c"
# include "tokenizer.c"

typedef long double (*d2dFunc)(long double);

struct Literal {
    long double value;
};

struct Identifier {
    char *name;
};

struct Expr2 {
    struct Expression *lhs;
    struct Expression *rhs;
    char *op;
    // +, -, *, /, &&, ||, ==, !=, <, <=, >, >=, etc.
    // =, +=, -=, *=, /=, %=, &=, |=, ^=, <<=, >>=, etc.
};

struct Expr1 {
    struct Expression *expr;
    char *op;
    // possible unary operators: +, -, !, ~, etc.
};

struct Builtin {
    d2dFunc func;
    char *name;
    struct Expression *expr;
};

// abs, sin, cos, tan, asin, acos, atan, sqrt, log, log10, exp, ceil, floor, round, etc.
struct Expression {
    enum Tag tag;

    union {
        struct Literal *literal;
        struct Identifier *identifier;
        struct Expr2 *expr2;
        struct Builtin *builtin;
        struct Expr1 *expr1;
        struct None *none;
    };
};

// struct Print {
//     char *message;
//     struct Expression expr;
// };
/// @attention Too difficult to implement, just ignore it

struct Function {
    struct Identifier func;
    struct Expression *expr; // ends with TNull
};

struct Return {
    struct Expression *expr;
};

struct Block {
    struct Statement *stmts; // ends with TNull
};

struct While {
    struct Expression *cond;
    struct Block *block;
};

struct If {
    struct Expression *cond;
    struct Block *then_block;
    struct Block *else_block; // may be TNull
};

struct Statement {
    enum Tag tag;

    union {
        struct Expression *expr;
        struct Builtin *builtin;
        // struct Print print;
        // struct Function function; // may not be implemented
        // struct Return ret;
        struct Block *block;
        struct If *if_stmt;
        struct While *while_stmt;
    };
};

long double prt(const long double x) {
    return printf("%Lf", x);
}

d2dFunc get_func(const char *name) {
    if (strstr(name, "abs")) {
        return fabsl;
    } else if (strstr(name, "sin")) {
        return sinl;
    } else if (strstr(name, "cos")) {
        return cosl;
    } else if (strstr(name, "tan")) {
        return tanl;
    } else if (strstr(name, "asin")) {
        return asinl;
    } else if (strstr(name, "acos")) {
        return acosl;
    } else if (strstr(name, "atan")) {
        return atanl;
    } else if (strstr(name, "sqrt")) {
        return sqrtl;
    } else if (strstr(name, "log")) {
        return logl;
    } else if (strstr(name, "log10")) {
        return log10l;
    } else if (strstr(name, "exp")) {
        return expl;
    } else if (strstr(name, "ceil")) {
        return ceill;
    } else if (strstr(name, "floor")) {
        return floorl;
    } else if (strstr(name, "round")) {
        return roundl;
    } else if (strstr(name, "prt")) {
        return prt;
    } else {
        return NULL;
    }
}

void Expr_delete(struct Expression *expr) {
    if (expr == NULL) {
        return;
    }
    switch (expr->tag) {
        case TLiteral:
            break;
        case TIdentifier:
            free(expr->identifier->name);
            free(expr->identifier);
            break;
        case TExpr2:
            Expr_delete(expr->expr2->lhs);
            Expr_delete(expr->expr2->rhs);
            free(expr->expr2->op);
            free(expr->expr2);
            break;
        case TExpr1:
            Expr_delete(expr->expr1->expr);
            free(expr->expr1->op);
            free(expr->expr1);
            break;
        case TBuiltin:
            Expr_delete(expr->builtin->expr);
            free(expr->builtin->name);
            free(expr->builtin);
            break;
        default:
            break;
    }
    free(expr);
}

void Block_delete(struct Block *block);

void Stmt_delete(struct Statement *stmt) {
    if (stmt == NULL) {
        return;
    }
    switch (stmt->tag) {
        case TExpression:
            Expr_delete(stmt->expr);
            break;
        case TBlock:
            Block_delete(stmt->block);
            break;
        case TIf:
            Expr_delete(stmt->if_stmt->cond);
            Block_delete(stmt->if_stmt->then_block);
            Block_delete(stmt->if_stmt->else_block);
            free(stmt->if_stmt);
            break;
        case TWhile:
            Expr_delete(stmt->while_stmt->cond);
            Block_delete(stmt->while_stmt->block);
            free(stmt->while_stmt);
            break;
        default:
            break; // this should not happen
    }
    free(stmt);
}

void Block_delete(struct Block *block) {
    if (block == NULL) {
        return;
    }
    struct Statement *stmt = block->stmts;
    while (stmt->tag != TNull) {
        Stmt_delete(stmt);
        stmt++;
    }
    free(block);
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

int operator_priority(const char *op) {
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

struct Expression *parse_expression(struct TokenData *tokens) {
    struct Expression *expr[STACK_SIZE];
    int expr_top = 0;
    char op[STACK_SIZE][3];
    int op_top = 0;

    int brace = 0;
    int flag = 0; // use for if and while ( condition ), process until )

    struct Token token = Ts_pop(tokens);

    if (token.tag == TokenNull) {
        struct Expression *result = malloc(sizeof(struct Expression));
        result->tag = TNull;
        return result;
    }
    if (token.tag == TokenOperator && token.token[0] == '(') {
        flag = 1;
    }
    while (token.tag != TokenNull && token.tag != TokenLineSep) {
        if (token.tag == TokenNumber) {
            struct Expression *expression = malloc(sizeof(struct Expression));
            expression->tag = TLiteral;
            expression->literal = malloc(sizeof(struct Literal));
            expression->literal->value = strtold(token.token, NULL);
            expr[expr_top++] = expression;
        } else if (token.tag == TokenWord) {
            // Tell if it is function call or variable
            if (*Ts_peek(tokens).token == '(') {
                // a function call
                struct Builtin *builtin = malloc(sizeof(struct Builtin));
                builtin->name = malloc(strlen(token.token) + 1);
                strcpy(builtin->name, token.token);
                builtin->func = get_func(token.token);
                builtin->expr = parse_expression(tokens);
                struct Expression *expression = malloc(sizeof(struct Expression));
                expression->tag = TBuiltin;
                expression->builtin = builtin;
                expr[expr_top++] = expression;
            } else {
                // a variable
                struct Identifier *identifier = malloc(sizeof(struct Identifier));
                identifier->name = malloc(strlen(token.token) + 1);
                strcpy(identifier->name, token.token);
                struct Expression *expression = malloc(sizeof(struct Expression));
                expression->tag = TIdentifier;
                expression->identifier = identifier;
                expr[expr_top++] = expression;
            }
        } else if (token.tag == TokenOperator) {
            if (token.token[0] == '(') {
                brace++;
            } else if (token.token[0] == ')') {
                brace--;
                while (op_top > 0) {
                    struct Expression *expression = malloc(sizeof(struct Expression));
                    expression->tag = TExpr2;
                    expression->expr2 = malloc(sizeof(struct Expr2));
                    expression->expr2->rhs = expr[--expr_top]; // stack, the top is the right side
                    expression->expr2->lhs = expr[--expr_top];
                    expression->expr2->op = malloc(3 * sizeof(char));
                    memcpy(expression->expr2->op, op[--op_top], 3);
                    expr[expr_top++] = expression;
                }
                op_top--; // pop the (
                if (flag && brace == 0) {
                    break;
                }
            } else {
                while (op_top > 0 && operator_priority(op[op_top - 1]) >= operator_priority(token.token)) {
                    struct Expression *expression = malloc(sizeof(struct Expression));
                    expression->tag = TExpr2;
                    expression->expr2->rhs = expr[--expr_top]; // stack, the top is the right side
                    expression->expr2->lhs = expr[--expr_top];
                    expression->expr2->op = malloc(3 * sizeof(char));
                    memcpy(expression->expr2->op, op[--op_top], 3);
                    expr[expr_top++] = expression;
                }
                strcpy(op[op_top], token.token);
                op_top++;
            }
        }
        token = Ts_pop(tokens);
    }
    while (op_top > 0) {
        struct Expression *expression = malloc(sizeof(struct Expression));
        expression->tag = TExpr2;
        expression->expr2 = malloc(sizeof(struct Expr2));
        expression->expr2->rhs = expr[--expr_top];
        expression->expr2->lhs = expr[--expr_top];
        expression->expr2->op = malloc(3 * sizeof(char));
        memcpy(expression->expr2->op, op[--op_top], 3);
        expr[expr_top++] = expression;
    }
    if (expr_top == 1) {
        return expr[0];
    } else {
        struct Expression *result = malloc(sizeof(struct Expression));
        result->tag = TError;
        return result;
    }
};

struct Statement *parse_statement(struct TokenData *tokens) {
    struct Token token = Ts_peek(tokens);
    struct Statement *stmt = malloc(sizeof(struct Statement));
    if (token.tag == TokenWord) {
        if (strstr(token.token, "if")) {
            Ts_pop(tokens);
            struct Expression *cond = parse_expression(tokens);
            struct Block *then_block = parse_block(tokens);
            struct Token is_else = Ts_peek(tokens);
            struct Block *else_block;
            if (is_else.tag == TokenWord && strstr(is_else.token, "else")) {
                Ts_advance(tokens);
                else_block = parse_block(tokens);
            } else {
                else_block = malloc(sizeof(struct Block));
                else_block->stmts = malloc(sizeof(struct Statement));
                else_block->stmts->tag = TNull;
            }
            stmt->tag = TIf;
            stmt->if_stmt = malloc(sizeof(struct If));
            stmt->if_stmt->cond = cond;
            stmt->if_stmt->then_block = then_block;
            stmt->if_stmt->else_block = else_block;
            // consume the newline, make sure
        } else if (strstr(token.token, "while")) {
            Ts_pop(tokens);
            struct Expression *cond = parse_expression(tokens);
            struct Block *block = parse_block(tokens);
            stmt->tag = TWhile;
            stmt->while_stmt->cond = cond;
            stmt->while_stmt->block = block;
            // consume the newline, make sure
        } else {
            const struct Expression *expr = parse_expression(tokens);
            stmt->tag = TExpression;
            stmt->expr = expr;
        }
    }

    // consume the newline last
    while (Ts_peek(tokens).tag == TokenLineSep) {
        Ts_advance(tokens);
    }
    return stmt;
}

struct Block *parse_block(struct TokenData *tokens) {
    int flag = 0;
    int brace = 0;
    struct Token token = Ts_peek(tokens);
    if (token.tag == TokenOperator && token.token[0] == '{') {
        flag = 1;
        Ts_advance(tokens);
        brace++;
    }
    // use for { block }, process until }
    // if flag is 0, process whole file, until TokenNull
    struct Block *block = malloc(sizeof(struct Block));
    block->stmts = malloc(sizeof(struct Statement) * STACK_SIZE);
    int index = 0;
    while (1) {
        struct Statement *stmt = parse_statement(tokens);
        block->stmts[index++] = *stmt;
        if (stmt->tag == TNull) {
            break;
        }
        token = Ts_peek(tokens);
        if (token.tag == TokenOperator && token.token[0] == '{') {
            brace++;
        }
        if (token.tag == TokenOperator && token.token[0] == '}') {
            brace--;
            Ts_advance(tokens); // consume the }
            if (flag && brace == 0) {
                break;
            }
        }
    }
    block->stmts[index].tag = TNull;
    return block;
}

struct Block *parse_file(struct TokenData *tokens) {
    // free tokens
    struct Block *block = parse_block(tokens);
    Ts_delete(tokens);
    return block;
}
