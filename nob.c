#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

static Cmd cmd_;
static Cmd *cmd = &cmd_;

#define SRC "src/"
#define BUILD "build/"
#define TESTS "tests/"
#define RUNNERS TESTS "runners/"

void cmd_cc_common(void)
{
#if defined(_MSC_VER) && !defined(__clang__)
    cmd_append(cmd, "cl");
    cmd_append(cmd, "-nologo");
    cmd_append(cmd, "-std:c11");
    cmd_append(cmd, "-W4");
    cmd_append(cmd, "-Od");
    cmd_append(cmd, "-Zi");
    cmd_append(cmd, "-D_CRT_SECURE_NO_WARNINGS");
#else
    cmd_append(cmd, "cc");
    cmd_append(cmd, "-std=c11");
    cmd_append(cmd, "-Wall");
    cmd_append(cmd, "-Wextra");
    cmd_append(cmd, "-O0");
    cmd_append(cmd, "-ggdb");
#endif
    // cmd_append(cmd, "/fsanitize=address");
}

void append_test(void)
{
    cmd_cc_common();
    cmd_append(cmd, "-DUNITY_INCLUDE_DOUBLE");
    cmd_append(cmd, TESTS "unity.c");
}

void cmd_cc_output(const char *output)
{
#if defined(_MSC_VER) && !defined(__clang__)
    cmd_append(cmd, temp_sprintf("/Fe:%s", output));
#else
    cmd_append(cmd, "-o", output);
#endif
}

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    bool make_calculator = true;
    bool run = true;
    bool make_tests = true;

    if (make_calculator)
    {
        if (!mkdir_if_not_exists(BUILD)) return 1;

        cmd_cc_common();
        cmd_append(cmd, "-I..\\src");
        cmd_append(cmd, SRC "calculator.c");
        cmd_append(cmd, SRC "tokenizer.c");
        cmd_append(cmd, SRC "parser.c");
        cmd_cc_output(BUILD "calculator.exe");
        cmd_append(cmd, "-lm");

        if (!cmd_run(cmd)) return 1;

        if (run)
        {
            cmd_append(cmd, BUILD "calculator.exe");
            cmd_append(cmd, "-print-infix");
            // cmd_append(cmd, "-print-rpn");
            // cmd_append(cmd, "-print-s");
            cmd_append(cmd, "-input=(1 + 2*(3 - 4^0))/7 - 5^2");
            if (!cmd_run(cmd)) return 1;
        }
    }

    if (make_tests)
    {
        if (!mkdir_if_not_exists(RUNNERS)) return 1;

        append_test();
        cmd_append(cmd, SRC "tokenizer.c");
        cmd_append(cmd, TESTS "test_tokenizer.c");
        cmd_cc_output(RUNNERS "test_tokenizer.test.exe");
        if (!cmd_run(cmd)) return 1;

        append_test();
        cmd_append(cmd, SRC "tokenizer.c");
        cmd_append(cmd, SRC "parser.c");
        cmd_append(cmd, TESTS "test_parser.c");
        cmd_cc_output(RUNNERS "test_parser.test.exe");
        cmd_append(cmd, "-lm");

        if (!cmd_run(cmd)) return 1;

        if (run)
        {
            cmd_append(cmd, RUNNERS "test_tokenizer.test.exe");
            if (!cmd_run(cmd)) return 1;

            cmd_append(cmd, RUNNERS "test_parser.test.exe");
            if (!cmd_run(cmd)) return 1;
        }
    }


    return 0;
}
