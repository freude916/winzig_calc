# pragma once
# ifndef TOKENIZER_H
# define TOKENIZER_H
# include <stdio.h>
# include <stdlib.h>
# include <string.h>

# include "defs.h"


/// TokenData.constructor
struct TokenData *Ts_create() {
    struct TokenData *tokens = malloc(sizeof(struct TokenData));
    void *new_memory = malloc(sizeof(struct Token) * INIT_TOKEN_COUNT);
    if (!new_memory) {
        panic("out of memory!", 1) // out of memory!!!
    }
    memset(new_memory, -1, sizeof(struct Token) * INIT_TOKEN_COUNT); // FIXME: for debug
    tokens->tokens = new_memory;
    tokens->count = 0;
    tokens->size = INIT_TOKEN_COUNT;
    tokens->error = Running;
    tokens->index = 0;
    return tokens;
}

/// TokenData.push: push a token
void Ts_push(struct TokenData *tokens, const enum DataTag tag, const char *const token, unsigned long token_len) {
    if (!tokens) {
        return;
    }
    if (token_len <= 0) {
        token_len = strlen(token);
    }
    if (tokens->count >= tokens->size) {
        tokens->size *= 2;
        void* new_memory = realloc(tokens->tokens, sizeof(struct Token) * tokens->size);
        if (!new_memory) {
            panic("out of memory!", 1);
        }
        tokens->tokens = new_memory;
    }

    tokens->tokens[tokens->count].tag = tag;
    tokens->tokens[tokens->count].token = malloc((token_len + 1) * sizeof(char));
    strcpy(tokens->tokens[tokens->count].token, token);
    tokens->count++;
}

/// TokenData.end: end the token list, submit
void Ts_end(struct TokenData *tokens) {
    Ts_push(tokens, TokenNull, "", 1);
    // it will never push then
    // void* new_memory = realloc(tokens->tokens, sizeof(struct Token) * (tokens->count + 2));
    // if (!new_memory) {
    //     panic("out of memory!", 1);
    // }
    // tokens->tokens = new_memory;
    if (tokens-> error == Running) {
        tokens->error = Success;
    }
}

/// TokenData.destructor
void Ts_delete(struct TokenData *tokens) {
    free(tokens->tokens);
    free(tokens);
}

void Ts_refresh(struct TokenData *tokens) {
    tokens->index = 0;
    tokens->count = 0;
    tokens->error = Running;
}

/// for parser. pop a token.
struct Token Ts_pop(struct TokenData *tokens) {
    if (tokens->index >= tokens->count) {
        tokens->error = UnexpectedEnd;
        struct Token token;
        token.tag = TokenNull;
        token.token = "";
        return token;
    }
    return tokens->tokens[tokens->index++];
}

/// for parser. peek a token for check, not increase the index.
struct Token Ts_peek(struct TokenData *tokens) {
    if (tokens->index >= tokens->count) {
        tokens->error = UnexpectedEnd;
        struct Token token;
        token.tag = TokenNull;
        token.token = "";
        return token; // we'd better keep this for further check
    }
    return tokens->tokens[tokens->index];
}

void Ts_advance(struct TokenData *tokens) {
    tokens->index++;
}

/**
 * Tokenize the source code
 *
 * @param tokens the token data
 * @param src the source code, better with space between tokens, if not also OK
 * @return a list of tokens (ends with "\0")
 */
void tokenize(struct TokenData *tokens, const char *src) {
    /// OK
    /// Passed test on 2024-11-06 22:07
    enum TokenType state = TokenNull;
    // Null for initial or blank, Word for word, Number for number, Operator for operator

    char *token = malloc(sizeof(char) * MAX_TOKEN_LEN); // assume never exceed MAX_TOKEN_LEN. what hell can be so long?
    unsigned long token_len = 0;
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
        return;
    }
    do {
        if (*src == ' ' || *src == '\t') {
            PUSH_TOKEN(state);
            state = TokenNull;
            continue;
        }
        if (*src == '\n' || *src == '\r' || *src == ';') {
            PUSH_TOKEN(state);
            Ts_push(tokens, TokenLineSep, ";", 1);
            while (*src == '\n' || *src == '\r' || *src == ';') src++;
            state = TokenNull;
            src--;
            continue;
        }
        // if (*src == '\n' || *src == '\r') {
        //     PUSH_TOKEN(state);
        //     state = TokenNull;
        //     continue;
        // }
        if (*src >= '0' && *src <= '9' || *src == '.') {
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
            if (*src == '=' && state == TokenOperator) {
                // all operator has a version with '=' :)
                PUSH_CHAR(*src);
                PUSH_TOKEN(TokenOperator);
                state = TokenNull;
            } else {
                if (state == TokenOperator) {
                    report_error(tokens->error, SyntaxError, "two operators in a row");
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
        if (*src != '\0') {
            report_error(tokens->error, InvalidChar, "invalid character");
            break;
        }
    } while (*++src != '\0');
    trace();
    PUSH_TOKEN(state);
    Ts_end(tokens);
    free(token);
#undef PUSH_CHAR
#undef PUSH_TOKEN
}
# endif