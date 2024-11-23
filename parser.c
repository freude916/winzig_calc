# pragma once
# ifndef PARSER_H
# define PARSER_H
# include <stdlib.h>
# include <string.h>
# include "mathfuns.c"
# include "defs.h"

struct Expression* Literal_create(const long double value) {
    struct Expression *expression = malloc(sizeof(struct Expression));
    expression->tag = GLiteral;
    expression->literal = malloc(sizeof(struct Literal));
    expression->literal->value = value;
    return expression;
}

struct Expression* Identifier_create(const char *name) {
    struct Expression *expression = malloc(sizeof(struct Expression));
    expression->tag = GIdentifier;
    expression->identifier = malloc(sizeof(struct Identifier));
    expression->identifier->name = malloc(strlen(name) + 1);
    strcpy(expression->identifier->name, name);
    return expression;
}

struct Expression* Expr2_create(struct Expression *lhs, struct Expression *rhs, const char *op) {
    struct Expression *expression = malloc(sizeof(struct Expression));
    expression->tag = GExpr2;
    expression->expr2 = malloc(sizeof(struct Expr2));
    expression->expr2->lhs = lhs;
    expression->expr2->rhs = rhs;
    expression->expr2->op = malloc(3 * sizeof(char));
    strcpy(expression->expr2->op, op);
    return expression;
}

struct Expression* Builtin_create(const char *name, struct Expression *expr) {
    struct Expression *expression = malloc(sizeof(struct Expression));
    expression->tag = GBuiltin;
    expression->builtin = malloc(sizeof(struct Builtin));
    expression->builtin->name = malloc(strlen(name) + 1);
    strcpy(expression->builtin->name, name);
    expression->builtin->func = get_func(name);
    expression->builtin->expr = expr;
    return expression;
}

/// Expression.destructor
void Expr_delete(struct Expression *expr) {
    if (expr == NULL) {
        return;
    }
    switch (expr->tag) {
        case GLiteral:
            break;
        case GIdentifier:
            free(expr->identifier->name);
            free(expr->identifier);
            break;
        case GExpr2:
            Expr_delete(expr->expr2->lhs);
            Expr_delete(expr->expr2->rhs);
            free(expr->expr2->op);
            free(expr->expr2);
            break;
        case GExpr1:
            Expr_delete(expr->expr1->expr);
            free(expr->expr1->op);
            free(expr->expr1);
            break;
        case GBuiltin:
            Expr_delete(expr->builtin->expr);
            free(expr->builtin->name);
            free(expr->builtin);
            break;
        default:
            break;
    }
    free(expr);
}

/// Statement.destructor
void Statement_delete(struct Statement *stmt) {
    if (stmt == NULL) {
        return;
    }
    switch (stmt->tag) {
        case GExpression:
            Expr_delete(stmt->expr);
            break;
        case GBlock:
            Block_delete(stmt->block);
            break;
        case GIf:
            Expr_delete(stmt->if_stmt->cond);
            Block_delete(stmt->if_stmt->then_block);
            Block_delete(stmt->if_stmt->else_block);
            free(stmt->if_stmt);
            break;
        case GWhile:
            Expr_delete(stmt->while_stmt->cond);
            Block_delete(stmt->while_stmt->block);
            free(stmt->while_stmt);
            break;
        default:
            break; // this should not happen
    }
    free(stmt);
}

/// Block.destructor
void Block_delete(struct Block *block) {
    if (block == NULL) {
        return;
    }
    struct Statement **stmt = block->stmts;
    while (stmt[0]->tag != GNull) {
        Statement_delete(*stmt);
        stmt++;
    }
    free(block);
}

struct Parser* Parser_create() {
    struct Parser *parser = malloc(sizeof(struct Parser));
    parser->error = Running;
    return parser;
}

/**
 * Get the priority of the operator.
 *
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

/// Parse an expression
/// @param parser: the parser object
/// @param tokens: the token data
/// @param brace_flag: 1 for ( expr ), 0 for whole line
struct Expression *parse_expression(struct Parser *parser, struct TokenData *tokens, const int brace_flag) {
    struct Expression *exps[STACK_SIZE];
    int expr_top = 0;
    char ops[STACK_SIZE][3];
    int op_top = 0;

    int brace = 0;
    // use for if and while ( condition ), process until )

    struct Token token = Ts_pop(tokens);

# define EPush(expr) if (expr_top < STACK_SIZE) exps[expr_top++] = expr; else report_error(parser->error, TooComplexGrammar, "too complex expression")
# define EPop2(lhs, rhs) \
    if (expr_top > 1){ \
        rhs = exps[--expr_top]; \
        lhs = exps[--expr_top]; \
    }else{ \
        report_error(parser->error, UnexpectedEnd, "expr: unexpected end"); \
    break; \
} \

# define OpPush(op) if (op_top < STACK_SIZE) strcpy(ops[op_top++], op); else report_error(parser->error, TooComplexGrammar, "too complex expression")
# define OpPop(op) \
    if (op_top > 0){ \
        op = malloc(3 * sizeof(char)); \
        memcpy(op, ops[--op_top], 3); \
    }else{ \
        report_error(parser->error, UnexpectedEnd, "op: unexpected end"); \
        break; \
    } \

# define calc_once() {\
    struct Expression *expression = malloc(sizeof(struct Expression));\
    expression->tag = GExpr2;\
    expression->expr2 = malloc(sizeof(struct Expr2));\
    EPop2(expression->expr2->lhs, expression->expr2->rhs);\
    OpPop(expression->expr2->op);\
    EPush(expression);\
    }\

    if (token.tag == TokenNull) {
        struct Expression *result = malloc(sizeof(struct Expression));
        result->tag = GNull;
        return result;
    }
    // if (token.tag == TokenOperator && token.token[0] == '(') {
    //     brace_flag = 1;
    // }
    while (token.tag != TokenNull && token.tag != TokenLineSep && tokens->error == Success) {
        if (token.tag == TokenNumber) {
            EPush(Literal_create(strtold(token.token, nullptr)));
        } else if (token.tag == TokenWord) {
            // Tell if it is function call or variable
            if (*Ts_peek(tokens).token == '(') {
                // a function call
                struct Expression* expression = Builtin_create(token.token, parse_expression(parser, tokens, 1));
                EPush(expression);
            } else {
                // a variable
                EPush(Identifier_create(token.token));
            }
        } else if (token.tag == TokenOperator) {
            if (token.token[0] == '(') {
                brace++;
            } else if (token.token[0] == ')') {
                brace--;
                while (op_top > 0 && ops[op_top][0] != '(') calc_once();
                // 1: at least one ( in stack
                // op_top--; // wrong, didn't push (
                if (brace_flag && brace == 0) break;
            } else if  (token.token[0] == '}'){
                break;
            }else {
                while (op_top > 0 && operator_priority(ops[op_top - 1]) >= operator_priority(token.token)) calc_once();
                OpPush(token.token);
            }
        }
        token = Ts_pop(tokens);
    }
    trace();
    while (op_top > 0) {
        calc_once();
    }
    if (expr_top == 1) {
        return exps[0];
    } else {
        struct Expression *result = malloc(sizeof(struct Expression));
        result->tag = GError;
        report_error(parser->error, SyntaxError, "didn't process all expressions");
        return result;
    }
};

/// Parse a statement
struct Statement *parse_statement(struct Parser *parser, struct TokenData *tokens) {
    struct Token token = Ts_peek(tokens);
    struct Statement *stmt = malloc(sizeof(struct Statement));
    if (token.tag == TokenWord) {
        if (strstr(token.token, "if")) {
            Ts_pop(tokens);
            stmt->tag = GIf;
            stmt->if_stmt = malloc(sizeof(struct If));
            stmt->if_stmt->cond = parse_expression(parser,tokens, 1);
            stmt->if_stmt->then_block = parse_block(parser,tokens, 1);
            const struct Token is_else = Ts_peek(tokens);
            if (is_else.tag == TokenWord && strstr(is_else.token, "else")) {
                Ts_advance(tokens);
                stmt->if_stmt->else_block = parse_block(parser,tokens, 1);
            } else {
                stmt->if_stmt->else_block = malloc(sizeof(struct Block));
                stmt->if_stmt->else_block->stmts = malloc(sizeof(struct Statement *));
                stmt->if_stmt->else_block->stmts[0] = malloc(sizeof(struct Statement));
                stmt->if_stmt->else_block->stmts[0]->tag = GNull;
            }
            // consume the newline, make sure
        } else if (strstr(token.token, "while")) {
            Ts_pop(tokens);
            stmt->tag = GWhile;
            stmt->while_stmt = malloc(sizeof(struct While));
            stmt->while_stmt->cond = parse_expression(parser, tokens, 1);
            stmt->while_stmt->block = parse_block(parser, tokens, 1);
            // consume the newline, make sure
        } else {
            stmt->tag = GExpression;
            stmt->expr = parse_expression(parser, tokens, 0);
        }
    }else if (token.tag == TokenNumber){
        stmt->tag = GExpression;
        stmt->expr = parse_expression(parser, tokens, 0);
    }else if (token.tag == TokenOperator) {
        if (token.token[0] == '{') {
            parse_block(parser, tokens, 1);
        }
        stmt->tag = GExpression;
        stmt->expr = parse_expression(parser, tokens, 0);
    }else if (token.tag == TokenNull) {
        stmt->tag = GNull;
    }

    return stmt;
}

/// Parse a block
struct Block *parse_block(struct Parser *parser, struct TokenData *tokens, const int inner) {
    int brace = 0;
    struct Token token = Ts_peek(tokens);
    if (token.tag == TokenOperator && token.token[0] == '{') {
        Ts_advance(tokens);
        token = Ts_peek(tokens);
        if (token.tag == TokenLineSep) {
            Ts_advance(tokens);
        }
        brace++;
    }
    // use for { block }, process until }
    // if flag is 0, process whole file, until TokenNull
    struct Block *block = malloc(sizeof(struct Block));
    block->stmts = malloc(sizeof(struct Statement) * STACK_SIZE);
    int index = 0;
    while (1) {
        struct Statement *stmt = parse_statement(parser, tokens);
        block->stmts[index++] = stmt;
        if (stmt->tag == GNull || Ts_peek(tokens).tag == TokenNull) {
            break;
        }
        while ((token = Ts_peek(tokens)).tag == TokenLineSep) {
            Ts_advance(tokens);
        }

        if (token.tag == TokenOperator && token.token[0] == '{') {
            brace++;
        }
        if (token.tag == TokenOperator && token.token[0] == '}') {
            brace--;
            Ts_advance(tokens); // consume the }
            if (inner && brace == 0) {
                break;
            }
        }
    }
    block->stmts[index] = malloc(sizeof(struct Statement));
    block->stmts[index]->tag = GNull;
    return block;
}

/// Parse a file
void parse_file(struct Parser *parser, struct TokenData *tokens) {
    // free tokens
    struct Block *block = parse_block(parser, tokens, 0);
    if (parser->error == Running) {
        parser->error = Success;
    }
    parser->result_block = block;
}

/// Parser.destructor
void Parser_delete(struct Parser *parser) {
    free(parser);
}

/// Parser.refresh
void Parser_refresh(struct Parser *parser) {
    parser->error = Running;
    Block_delete(parser->result_block);
    parser->result_block = nullptr;
}
# endif