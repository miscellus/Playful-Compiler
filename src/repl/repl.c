/*

Build

# Windows MSVC
cl -D_CRT_SECURE_NO_WARNINGS -W4 User32.Lib crossline.c example.c /Feexample.exe

# Windows Clang
clang -D_CRT_SECURE_NO_WARNINGS -Wall -lUser32 crossline.c example.c -o example.exe

# Linux Clang
clang -Wall crossline.c example.c -o example

# GCC(Linux, MinGW, Cygwin, MSYS2)
gcc -Wall crossline.c example.c -o example

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "external/crossline.h"

#include "parser.h"
#include "tokenizer.h"
#include "var_table.h"

int main ()
{
    VarTable variables = {0};
    char buf[1024];

    // crossline_completion_register(completion_hook);
    crossline_history_load("playful_history.txt");

    while (crossline_readline(">>> ", buf, sizeof(buf)))
    {
        TokenStream ts = TokenStreamFromCStr(buf);
        Expr *expr = ParseExprSeq(&ts);
        if (expr == NULL) continue;

        // PrintExpr(expr);

        double result = EvalExpr(&variables, expr);
        printf("%g\n", result);
    }

    crossline_history_save("playful_history.txt");
    return 0;
}