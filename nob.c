#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

static Cmd cmd;

#define SRC "src/"
#define BUILD "build/"

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    if (!mkdir_if_not_exists(BUILD)) return 1;

    if (true)
    {

        cmd_append(&cmd, "cl");

        cmd_append(&cmd, "/nologo");
        cmd_append(&cmd, "/D_CRT_SECURE_NO_WARNINGS");

        cmd_append(&cmd, "/W4");
        cmd_append(&cmd, "/std:c11");
        cmd_append(&cmd, "/Od");
        cmd_append(&cmd, "/Zi");

        // cmd_append(&cmd, "/fsanitize=address");

        cmd_append(&cmd, "/I..\\src");

        cmd_append(&cmd, SRC "calculator.c");
        cmd_append(&cmd, SRC "tokenizer.c");
        cmd_append(&cmd, SRC "parser.c");

        cmd_append(&cmd, "/Fe:" BUILD "calculator.exe");

        if (!cmd_run(&cmd)) return 1;
    }

    if (false)
    {
        cmd_append(&cmd, BUILD "calculator.exe");
        // cmd_append(&cmd, "-print-infix");
        // cmd_append(&cmd, "-print-rpn");
        // cmd_append(&cmd, "-print-s");
        cmd_append(&cmd, "-input=(1 + 2*(3 - 4^0))/7 - 5^2");
        if (!cmd_run(&cmd)) return 1;
    }

    return 0;
}
