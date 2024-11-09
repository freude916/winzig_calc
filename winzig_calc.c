# include <stdio.h>
# include "tokenizer.c"
# include "parser.c"
# include "interpreter.c"

struct WinzigCalc {
    struct TokenData *tokens;
    struct Parser *parser;
    struct Interpreter *interpreter;
    enum Error error;
};

struct WinzigCalc *WinzigCalc_create() {
    struct WinzigCalc *calc = malloc(sizeof(struct WinzigCalc));
    calc->tokens = Ts_create();
    calc->parser = Parser_create();
    calc->interpreter = Interpreter_create();
    return calc;
}

void WinzigCalc_delete(struct WinzigCalc *calc) {
    Ts_delete(calc->tokens);
    Parser_delete(calc->parser);
    Interpreter_delete(calc->interpreter);
    free(calc);
}

void winzig_inline(struct WinzigCalc *calc) {
    /// read 1 line and execute, then reset errors
    Ts_refresh(calc->tokens);
    Parser_refresh(calc->parser);
    Interpreter_refresh(calc->interpreter);

    printf(">>> ");
    fflush(stdout);
    char buf[256];
    fgets(buf, 256, stdin);

    // check keyboard interrupt
    if (strstr(buf, "exit")) {
        calc->error = KeyboardInterrupt;
        return;
    }

    tokenize(calc->tokens, buf);
    calc -> error = calc->tokens->error;
    if (calc->error != Success) return;

    parse_file(calc->parser, calc->tokens);
    calc -> error = calc->parser->error;
    if (calc->error != Success) return;

    long double result = interpret_file(calc->interpreter, calc->parser->result_block);
    calc -> error = calc->interpreter->error;
    if (calc->error != Success) return;

    printf("%Lf\n", result);

}

void winzig_repl(struct WinzigCalc *calc) {
    while (1) {
        winzig_inline(calc);
        if (calc->error == KeyboardInterrupt) {
            break;
        }
    }
}

void winzig_code(struct WinzigCalc *calc, char* code) {
    tokenize(calc->tokens, code);
    if (calc->tokens->error != Success) {
        // printf("Tokenize error: %d\n", calc->tokens->error); // reported error inside.
        return;
    }
    parse_file(calc->parser, calc->tokens);
    if (calc->parser->error != Success) {
        // printf("Parse error: %d\n", calc->parser->error);// reported error inside.
        return;
    }
    interpret_file(calc->interpreter, calc->parser->result_block);
    if (calc->interpreter->error != Success) {
        // printf("Interpret error: %d\n", calc->interpreter->error); // reported error inside.
        return;
    }
}

void winzig_file(struct WinzigCalc *calc, char* filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("Cannot open file %s\n", filename);
        return;
    }
    char buf[65536];
    fgets(buf, 65536, fp);
    winzig_code(calc, buf);
}

int winzig_ez_main(int argc, char *argv[]) {
    struct WinzigCalc *calc = WinzigCalc_create();
    if (argc == 1) {
        winzig_repl(calc);
    } else {
        winzig_file(calc, argv[1]);
    }
    WinzigCalc_delete(calc);
    return 0;
}