#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "options.h"

void
ExitPrintUsage(const char *program, int exitCode)
{
#define CL_OPTION_PRINT_FORMAT(optionStr, arg0, optionNum, description) "  %-24s %s\n"
#define CL_OPTION_PRINT_ARGS(optionStr, arg0, optionNum, description) , optionStr arg0, description

	fprintf(
	    stderr,
	    "Usage: %s [Options] <Expression>\n"
	    "Options:\n" CL_OPTION_LIST(CL_OPTION_PRINT_FORMAT),
	    program CL_OPTION_LIST(CL_OPTION_PRINT_ARGS));
	exit(exitCode);
}

Options
ParseCommandLineOptions(int argc, char const **argv)
{
	Options options = {0};

	options.program = (--argc, *argv++);

	bool needsMoreArguments = true;

	for (;;) {
		if (argc < 1) {
			if (needsMoreArguments)
				ExitPrintUsage(options.program, 1);
			else
				return options;
		}

		const char *argRest = NULL;
		enum OptionFlags flag = 0;

#define CL_OPTION_STRCMP(optionStr, arg0, optionNum, description) \
	if (strncmp((optionStr), *argv, sizeof(optionStr) - 1) == 0) { \
		argRest = *argv + sizeof(optionStr) - 1; \
		flag = CL_OPTION_##optionNum; \
	} \
	else
		CL_OPTION_LIST(CL_OPTION_STRCMP)
		{
			break;
		}

		options.flags |= flag;

		if (flag == CL_OPTION_INPUT_DIRECT) {
			options.input.direct = argRest;
			needsMoreArguments = false;
		}

		--argc;
		++argv;
	}

	if (options.flags & CL_OPTION_INPUT_DIRECT) {
		return options;
	}

	if (argc < 1) ExitPrintUsage(options.program, 1);

	const char *inputFile = (--argc, *argv++);

	if (strcmp("-", inputFile) == 0) {
		assert(!"TODO");
		options.input.file = stdin;
	}
	else {
		options.input.file = fopen(inputFile, "r");
		if (options.input.file == NULL) {
			fprintf(stderr, "Could not open file.\n");
			ExitPrintUsage(options.program, 1);
		}
	}

	return options;
}
