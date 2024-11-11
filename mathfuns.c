# pragma once
# include "defs.h"
# include <stdio.h>
# include <math.h>

long double my_print(struct Interpreter *interpreter, const long double x) {
    return (long double) printf("%Lf\n", x);
}

long double my_input(struct Interpreter *interpreter, const long double _) {
    /// read a long double from stdin
    long double x;
    char buf[256];
    char *end = nullptr;
    while (1){
        scanf("%s", buf);
        if (buf[0] == 'Q' || buf[0] == 'q'){
            interpreter -> error = KeyboardInterrupt;
            return 0.0;
        }
        x = strtold(buf, &end);
        if (*end != '\0' || end == buf) {
            printf("Not a valid number: %s\n", buf);
            printf("Input Q to exit current program\n");
        } else {
            break;
        }
    }
    return x;
}

# define quick_my(name, func) long double my_##name(struct Interpreter *interpreter, const long double x) { return func(x); }
quick_my(abs, fabsl)
quick_my(sin, sinl)
quick_my(cos, cosl)
quick_my(tan, tanl)
quick_my(asin, asinl)
quick_my(acos, acosl)
quick_my(atan, atanl)
quick_my(sqrt, sqrtl)
quick_my(log, logl)
quick_my(log10, log10l)
quick_my(exp, expl)
quick_my(ceil, ceill)
quick_my(floor, floorl)
quick_my(round, roundl)

/**
* Provide built-in function with name
* now provided: abs, sin, cos, tan, asin, acos, atan, sqrt, log, log10, exp, ceil, floor, round, etc.
*/
d2dFunc get_func(const char *name) {
# define check(label, func) if (strstr(name, label)) return func;
    check("abs", my_abs);
    check("sin", my_sin);
    check("cos", my_cos);
    check("tan", my_tan);
    check("asin", my_asin);
    check("acos", my_acos);
    check("atan", my_atan);
    check("sqrt", my_sqrt);
    check("log", my_log);
    check("log10", my_log10);
    check("exp", my_exp);
    check("ceil", my_ceil);
    check("floor", my_floor);
    check("round", my_round);
    check("print", my_print);
    check("input", my_input);
    return nullptr;
}