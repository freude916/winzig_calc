#include <stdio.h>
#include "calc.c"
// test parse_file

char my_src[] = "var = 2;\n"
                "if(var == 1){\n"
                " var = 3;\n"
                " vbr = 4;\n"
                "}"
                "prt(var);";
int main() {
    struct TokenData* tokens = tokenize(my_src);
    struct Block* block = parse_block(tokens);
    struct Runner* runner = Runner_create();
    Runner_evaluate(runner, block);
    return 0;
}