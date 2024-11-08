# pragma once

#define INIT_TOKEN_COUNT 1024
#define MAX_TOKEN_LEN 256
#define STACK_SIZE 1024
#define eps 1e-9

enum Tag {
    TNull, // nothing, only used in base, mark the end
    TNone, // nothing value, may appear in Expression?
    TError, // inside error happened

    TLiteral, TIdentifier,

    TExpr2, TExpr1, TExpression,

    TBuiltin, TPrint, TFunction,

    TStatement,

    TBlock,

    TWhile, TIf,
};