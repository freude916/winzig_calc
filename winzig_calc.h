# pragma once
# ifndef WINZIG_CALC_H
# define WINZIG_CALC_H
# include "base.h"

struct WinzigCalc {
    struct TokenData *tokens;
    struct Parser *parser;
    struct Interpreter *interpreter;
    enum Error error;
};

struct WinzigCalc *WinzigCalc_create();

void WinzigCalc_delete(struct WinzigCalc *calc);

void winzig_inline(struct WinzigCalc *calc);

void winzig_repl(struct WinzigCalc *calc);

int winzig_ez_main(int argc, char *argv[]);

void winzig_code(struct WinzigCalc *calc, char *code);

void winzig_file(struct WinzigCalc *calc, char *filename);

# endif //WINZIG_CALC_H
