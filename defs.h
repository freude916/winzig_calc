#pragma once


enum TokenType;
enum Tag;

struct None{};

struct Literal;
struct Identifier;

struct Expr2;
struct Expr1;
struct Expression;

struct Builtin;
struct Print;
struct Function;

struct Statement;

struct Block;

struct Token;
struct TokenData;

struct TokenData *Ts_create();
void Ts_push(struct TokenData *tokens, enum Tag tag, const char *token, int token_len);
void Ts_delete(struct TokenData *tokens);

void Ts_end(struct TokenData *tokens);

struct TokenData* tokenize(const char *src);

int operator_priority(const char* op);
struct Block* parse_file(struct TokenData* tokens); // free tokens
struct Block* parse_block(struct TokenData* tokens); // read from { to } or TNull
struct Statement* parse_statement(struct TokenData* tokens); // read until TokenNewline
struct Expression* parse_expression(struct TokenData* tokens); // read until TokenNewline
// struct Expression* parse_brace(struct TokenData* tokens); // read from ( to )

void Expression_eval(struct Expression* expr);
void Statement_eval(struct Statement* stmt);
void Block_eval(struct Block* block);
void evaluate(struct Block* block); // ? should it free blocks? will we reuse it?
