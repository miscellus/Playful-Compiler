#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdio.h>

#define CL_OPTION_LIST(X) \
	X("-print-infix", , PRINT_INFIX, "Print parenthesized expression with infix operators.") \
	X("-print-s", , PRINT_S, "Print parenthesized s-expression.") \
	X("-print-rpn", , PRINT_RPN, "Print expression in reverse polish notation (RPN).") \
	X("-input=", "<expression>", INPUT_DIRECT, "Directly passed input") \
	// END

#define CL_OPTION_ENUM_BIT_NUM(optionStr, arg0, optionNum, description) CL_OPTION_BIT_NUM_##optionNum,
enum ClOptionBitIndex {
	CL_OPTION_LIST(CL_OPTION_ENUM_BIT_NUM)
};

#define CL_OPTION_TYPE(optionStr, arg0, optionNum, description) \
	CL_OPTION_##optionNum = (1 << (CL_OPTION_BIT_NUM_##optionNum)),
enum OptionFlags {
	CL_OPTION_NONE = 0,
	CL_OPTION_LIST(CL_OPTION_TYPE)
};

typedef struct Options_t {
	const char *program;
	enum OptionFlags flags;

	union {
		FILE *file;
		const char *direct;
	} input;

} Options;

void
ExitPrintUsage(const char *program, int exitCode);

Options
ParseCommandLineOptions(int argc, char const **argv);

#endif