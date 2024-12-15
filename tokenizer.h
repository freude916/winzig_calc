# pragma once
# ifndef TOKENIZER_H
# define TOKENIZER_H
# include "base.h"

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
void Ts_refresh(struct TokenData *tokens);

void tokenize(struct TokenData *tokens, const char *src);

#endif //TOKENIZER_H
