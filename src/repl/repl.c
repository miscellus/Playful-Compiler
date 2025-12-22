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
#include "lexer.h"
#include "var_table.h"
#include "msc_arena.h"

int main ()
{
    VarTable variables = {0};
    char buf[1024];
    Parser parser = {0};
    parser.arena = msc_arena_create(64ull<<20ull);
    parser.arena.flags = MSC_ARENA_OPT_ZERO;

    // crossline_completion_register(completion_hook);
    crossline_history_load("playful_history.txt");

    while (crossline_readline(">>> ", buf, sizeof(buf)))
    {
        msc_arena_reset(&parser.arena);
        Lexer lex = LexerFromCStr(buf);
        parser.lex = &lex;
        Expr *expr = ParseExprSeq(&parser);
        if (expr == NULL) continue;

        // PrintExpr(expr);

        double result = EvalExpr(&variables, expr);
        printf("%g\n", result);
    }

    crossline_history_save("playful_history.txt");
    return 0;
}