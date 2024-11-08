# pragma once
# include <stdlib.h>
# include "defs.h"
# include "base.c"

enum TokenType {
    TokenNull, // used for state mark, don't use in real token
    TokenWord, TokenNumber, TokenOperator, TokenLineSep,
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

void Ts_push(struct TokenData *tokens, const enum Tag tag, const char *const token, int token_len) {
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

struct Token Ts_peek(struct TokenData *tokens) {
    if (tokens->index >= tokens->count) {
        struct Token token;
        token.tag = TokenNull;
        token.token = "";
        return token; // fuck, the parse_expression consumed the TokenNull, but we'd better keep this for further check
    }
    return tokens->tokens[tokens->index];
}

struct Token Ts_peek_next(struct TokenData *tokens) {
    if (tokens->index + 1 >= tokens->count) {
        struct Token token;
        token.tag = TokenNull;
        token.token = "";
        return token;
    }
    return tokens->tokens[tokens->index + 1];
}

void Ts_advance(struct TokenData *tokens) {
    tokens->index++;
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
        if (*src == ';') {
            PUSH_TOKEN(state);
            Ts_push(tokens, TokenLineSep, ";", 1);
            while (*src == '\n' || *src == '\r' || *src == ';') src++;
            state = TokenNull;
            src--;
            continue;
        }
        if (*src == '\n' || *src == '\r') {
            PUSH_TOKEN(state);
            state = TokenNull;
            continue;
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
            if (*src == '=' && state == TokenOperator) {
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
        if (*src != '\0') {
            tokens->error = 2; // invalid character
            break;
        }
    } while (*++src != '\0');
    PUSH_TOKEN(state);
    Ts_end(tokens);
    return tokens;
#undef PUSH_CHAR
#undef PUSH_TOKEN
}
