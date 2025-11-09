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

    const char *program = shift(argv, argc);

    bool build = true;
    bool run = false;
    bool test = false;

    while (argc) {
        char *arg = shift(argv, argc);

        if (strcmp(arg, "test") == 0) {
            build = false;
            test = true;
        }

        if (strcmp(arg, "run") == 0) {
            run = true;
        }
    }

    if (build)
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

    if (test)
    {
        if (!mkdir_if_not_exists(RUNNERS)) return 1;

        const char *test_tokenizer_exe = RUNNERS "test_tokenizer.test.exe";
        const char *test_parser_exe = RUNNERS "test_parser.test.exe";

        static const char *test_input_paths[] = {
            SRC "parser.c",
            SRC "parser.h",
            SRC "tokenizer.c",
            SRC "tokenizer.h",
            TESTS "test_parser.c",
            TESTS "test_tokenizer.c",
        };

        if (needs_rebuild(test_tokenizer_exe, test_input_paths, ARRAY_LEN(test_input_paths)))
        {
            nob_log(INFO, "Rebuilding tests");
            append_test();
            cmd_append(cmd, SRC "tokenizer.c");
            cmd_append(cmd, TESTS "test_tokenizer.c");
            cmd_cc_output(test_tokenizer_exe);
            if (!cmd_run(cmd)) return 1;

            append_test();
            cmd_append(cmd, SRC "tokenizer.c");
            cmd_append(cmd, SRC "parser.c");
            cmd_append(cmd, TESTS "test_parser.c");
            cmd_cc_output(test_parser_exe);
            cmd_append(cmd, "-lm");

            if (!cmd_run(cmd)) return 1;
        }

        nob_log(INFO, "Running tests");

        cmd_append(cmd, RUNNERS "test_tokenizer.test.exe");
        if (!cmd_run(cmd)) return 1;

        cmd_append(cmd, RUNNERS "test_parser.test.exe");
        if (!cmd_run(cmd)) return 1;
    }


    return 0;
}
