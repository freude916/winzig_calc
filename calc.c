#pragma once
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "defs.h"
#include "base.c"
#include "tokenizer.c"
#include "parser.c"
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

int string_hash(const char* str) {
    int hash = 5381;
    while (*str) {
        hash = (hash * 31 + *str) % 65536;
        str++;
    }
    return hash;
}

long double calc(long double a, long double b, char *op) {
    if (op[1] != '\0') {
        switch (*op) {
            case '>': return a >= b;
            case '<': return a <= b;
            case '=': return a == b;
            case '!': return a != b;
            default:  assert(0); // should not reach here
        }
    }
    switch (*op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/': return a / b;
        case '%': return fmod(a, b);
        case '^': return pow(a, b);
        case '&': return (long long)a & (long long)b;
        case '|': return (long long)a | (long long)b;
        case '>': return a > b;
        case '<': return a < b;
        default: assert(0); // should not reach here
    }
}

struct Executor {
    long double variables[65536]; // hash set, except to save 256 elements
    int error; // 0 for no err, 3 for runtime error
};

struct Executor* Executor_create() {
    struct Executor* executor = malloc(sizeof(struct Executor));
    memset(executor->variables, 0, sizeof(executor->variables));
    executor->error = 0;
    return executor;
}

long double Executor_get(struct Executor* executor, const char* name) {
    return executor->variables[string_hash(name)];
}

void Executor_set(struct Executor* executor, const char* name, long double value) {
    executor->variables[string_hash(name)] = value;
}

long double Executor_eval_Expression(struct Executor *executor, struct Expression *expr) {
    if (expr -> tag == TLiteral) {
        return expr -> literal -> value;
    } if (expr -> tag == TIdentifier) {
        return Executor_get(executor, expr -> identifier -> name); // the assignment should be done previously
    } if (expr -> tag == TBuiltin) {
        const long double value = Executor_eval_Expression(executor, expr -> builtin -> expr);
        return expr -> builtin -> func(value);
    } if (expr -> tag == TExpr2) {
        struct Expr2 *expr2 = expr -> expr2;
        if ((expr2->op[0] == '=' && expr2->op[1] == '\0') || (expr2->op[1] == '='  && expr2->op[0] != '=' && expr2->op[0] != '<' && expr2->op[0] != '>')) {
            if (expr2->lhs->tag == TIdentifier) {
                const long double value = Executor_eval_Expression(executor, expr2->rhs);
                if (expr2->op[1] == '=') {
                    // calc then assign
                    char op[2] = {expr2->op[0], '\0'};
                    long double before = Executor_get(executor, expr2->lhs->identifier->name);
                    long double after = calc(before, value, op);
                    Executor_set(executor, expr2->lhs->identifier->name, after);
                    return after;
                } else {
                    Executor_set(executor, expr2->lhs->identifier->name, value);
                    return value;
                }
            } else {
                executor->error = 3;
                return 0;
            }
        } else {
            return calc(Executor_eval_Expression(executor, expr2->lhs), Executor_eval_Expression(executor, expr2->rhs), expr2->op);
        }
    }
    // TODO: implement other tags
    assert(0);
}

void Executor_eval_Block(struct Executor *executor, struct Block *block) ;

void Executor_eval_Statement(struct Executor *executor, struct Statement *stmt) {
    if (stmt -> tag == TExpression) {
        Executor_eval_Expression(executor, stmt -> expr);
        return;
    } if (stmt -> tag == TIf) {
        const long double condition = Executor_eval_Expression(executor, stmt -> if_stmt -> cond);
        if (condition < eps) {
            Executor_eval_Block(executor, stmt -> if_stmt -> else_block);
        } else {
            Executor_eval_Block(executor, stmt -> if_stmt -> then_block);
        }
    } if (stmt -> tag == TWhile) {
        while (Executor_eval_Expression(executor, stmt -> while_stmt -> cond) > eps) {
            Executor_eval_Block(executor, stmt -> while_stmt -> block);
        }
    }
}

void Executor_eval_Block(struct Executor *executor, struct Block *block) {
    struct Statement* stmt = block -> stmts;
    while (stmt -> tag != TNull) {
        Executor_eval_Statement(executor, stmt);
        stmt++;
    }
}

void Executor_evaluate(struct Executor *executor, struct Block *block) {
    Executor_eval_Block(executor, block);
    Block_delete(block);
}