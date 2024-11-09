#pragma once
# ifndef INTERPRETER_H
# define INTERPRETER_H
#include <math.h>
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
* Define a function: fn <Function_Name>(<args...>){}; // MORE implement later
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

long double calc(struct Interpreter* interpreter, long double a, long double b, char *op) {
    if (op[1] != '\0') {
        switch (*op) {
            case '>': return a >= b;
            case '<': return a <= b;
            case '=': return a == b;
            case '!': return a != b;
            default:
                report_error(interpreter->error, RuntimeError, "Unknown operator");
                return 0; // should not reach here
        }
    }
    switch (*op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/': return a / b;
        case '%': return fmodl(a, b);
        case '^': return powl(a, b);
        case '&': return (long long)a & (long long)b;
        case '|': return (long long)a | (long long)b;
        case '>': return a > b;
        case '<': return a < b;
        default:
            report_error(interpreter->error, RuntimeError, "Unknown operator");
            return 0;
            // should not reach here
    }
}



struct Interpreter* Interpreter_create() {
    struct Interpreter* interpreter = malloc(sizeof(struct Interpreter));
    memset(interpreter->variables, -1, sizeof(interpreter->variables));
    interpreter->error = 0;
    return interpreter;
}

long double Interpreter_get(struct Interpreter* interpreter, const char* name) {
    long double value = interpreter->variables[string_hash(name)];
    if (isnanl(value)) {
        report_error(interpreter->error, MathError, "found an nan, this maybe undef variable or illegal operation");
    }
    return interpreter->variables[string_hash(name)];
}

void Interpreter_set(struct Interpreter* interpreter, const char* name, const long double value) {
    if (isnanl(value)) {
        report_error(interpreter->error, MathError, "found an nan from calculation, maybe you operated illegally");
    }
    interpreter->variables[string_hash(name)] = value;
}

long double interpret_Expression(struct Interpreter *interpreter, struct Expression *expr) {

    if (expr -> tag == GError) {
        report_error(interpreter->error, RuntimeError, "Uncaught error");
        return 0;
    }
    if (expr -> tag == GLiteral) {
        return expr -> literal -> value;
    } if (expr -> tag == GIdentifier) {
        return Interpreter_get(interpreter, expr -> identifier -> name); // the assignment should be done previously
    } if (expr -> tag == GBuiltin) {
        const long double value = interpret_Expression(interpreter, expr -> builtin -> expr);
        return expr -> builtin -> func(interpreter, value);
    } if (expr -> tag == GExpr2) {
        struct Expr2 *expr2 = expr -> expr2;
        if ((expr2->op[0] == '=' && expr2->op[1] == '\0') || (expr2->op[1] == '='  && expr2->op[0] != '=' && expr2->op[0] != '<' && expr2->op[0] != '>')) {
            if (expr2->lhs->tag == GIdentifier) {
                const long double value = interpret_Expression(interpreter, expr2->rhs);
                if (expr2->op[1] == '=') {
                    // calc then assign
                    char op[2] = {expr2->op[0], '\0'};
                    long double before = Interpreter_get(interpreter, expr2->lhs->identifier->name);
                    long double after = calc(interpreter, before, value, op);
                    Interpreter_set(interpreter, expr2->lhs->identifier->name, after);
                    return after;
                } else {
                    Interpreter_set(interpreter, expr2->lhs->identifier->name, value);
                    return value;
                }
            } else {
                interpreter->error = 3;
                return 0;
            }
        } else {
            return calc(interpreter, interpret_Expression(interpreter, expr2->lhs), interpret_Expression(interpreter, expr2->rhs), expr2->op);
        }
    }
    // MORE: implement other tags
    report_error(interpreter->error, RuntimeError, "Unknown expression tag");
    return 0;
}

long double interpret_Statement(struct Interpreter *interpreter, struct Statement *stmt) {

    if (stmt -> tag == GExpression) {
        return interpret_Expression(interpreter, stmt -> expr);
    } if (stmt -> tag == GIf) {
        const long double condition = interpret_Expression(interpreter, stmt -> if_stmt -> cond);
        if (condition < eps) {
            return interpret_Block(interpreter, stmt -> if_stmt -> else_block);
        } else {
            return interpret_Block(interpreter, stmt -> if_stmt -> then_block);
        }
    } if (stmt -> tag == GWhile) {
        while (interpret_Expression(interpreter, stmt -> while_stmt -> cond) > eps) {
            interpret_Block(interpreter, stmt -> while_stmt -> block);
        }
        return 0;
    }

}

long double interpret_Block(struct Interpreter *interpreter, struct Block *block) {
    struct Statement** stmt = block -> stmts;
    long double rv = 0;
    while (stmt[0] -> tag != GNull) {
        rv = interpret_Statement(interpreter, *stmt);
        stmt++;
    }
    return rv;
}

long double interpret_file(struct Interpreter *interpreter, struct Block *block) {
    long double rv = interpret_Block(interpreter, block);
    if (interpreter->error == Running) {
        interpreter->error = Success;
        return rv;
    }
    return nanl("");
}

void Interpreter_delete(struct Interpreter *interpreter) {
    free(interpreter);
}

void Interpreter_refresh(struct Interpreter *interpreter) {
    // memset(interpreter->variables, -1, sizeof(interpreter->variables)); // keep the variables in repl
    interpreter->error = Running;
}
# endif