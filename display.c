# pragma once
# ifndef DISPLAY_H
# define DISPLAY_H
# include "defs.h"
# include "parser.c"
# include <stdio.h>

void print_Statement(const struct Statement* statement);
void print_Block(const struct Block* block);

void print_Expression(const struct Expression* expression) {
    switch (expression->tag) {
        case GLiteral:
            printf("%Lf", expression->literal->value);
            break;
        case GIdentifier:
            printf("%s", expression->identifier->name);
            break;
        case GExpr2:
            printf("(");
            print_Expression(expression->expr2->lhs);
            printf(" %s ", expression->expr2->op);
            print_Expression(expression->expr2->rhs);
            printf(")");
            break;
        case GBuiltin:
            printf("%s(", expression->builtin->name);
            print_Expression(expression->builtin->expr);
            printf(")");
            break;
        default:
            printf("<unknown>");
    }
}

void print_Statement(const struct Statement* statement) {
    if (statement->tag == GExpression) {
        print_Expression(statement->expr);
        printf(";\n");
        return;
    }
    if (statement->tag == GIf) {
        printf("if");
        print_Expression(statement->if_stmt->cond);
        printf("{\n");
        print_Block(statement->if_stmt->then_block);
        printf("} else {\n");
        print_Block(statement->if_stmt->else_block);
        printf("}\n");
        return;
    }
    if (statement->tag == GWhile) {
        printf("while");
        print_Expression(statement->while_stmt->cond);
        printf("{\n");
        print_Block(statement->while_stmt->block);
        printf("}\n");
        return;
    }
    printf("<unknown>");
}

void print_Block(const struct Block* block) {
    for (int i = 0; block->stmts[i]->tag != GNull; i++) {
        print_Statement(block->stmts[i]);
    }
}



# endif