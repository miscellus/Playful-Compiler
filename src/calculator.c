#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tokenizer.h"
#include "parser.h"

#define CL_OPTION_LIST(X) \
	X("-print-infix" ,                , PRINT_INFIX  , "Print parenthesized expression with infix operators.") \
	X("-print-s"     ,                , PRINT_S      , "Print parenthesized s-expression.") \
	X("-print-rpn"   ,                , PRINT_RPN    , "Print expression in reverse polish notation (RPN).") \
	X("-input="      , "<expression>" , INPUT_DIRECT , "Directly passed input") \
	//END

#define CL_OPTION_ENUM_BIT_NUM(optionStr, arg0, optionNum, description) CL_OPTION_BIT_NUM_##optionNum,
enum ClOptionBitIndex
{
	CL_OPTION_LIST(CL_OPTION_ENUM_BIT_NUM)
};

#define CL_OPTION_TYPE(optionStr, arg0, optionNum, description) CL_OPTION_##optionNum = (1 << (CL_OPTION_BIT_NUM_##optionNum)),
enum OptionFlags
{
	CL_OPTION_NONE = 0,
	CL_OPTION_LIST(CL_OPTION_TYPE)
};

typedef struct Options_t
{
	const char *program;
	enum OptionFlags flags;

	union
	{
		FILE *file;
		const char *direct;
	} input;

} Options;

static void ExitPrintUsage(const char *program, int exitCode)
{
#define CL_OPTION_PRINT_FORMAT(optionStr, arg0, optionNum, description) "  %-24s %s\n"
#define CL_OPTION_PRINT_ARGS(optionStr, arg0, optionNum, description) ,optionStr arg0, description

	fprintf(stderr,
		"Usage: %s [Options] <Expression>\n"
		"Options:\n"
		CL_OPTION_LIST(CL_OPTION_PRINT_FORMAT)
		,
		program
		CL_OPTION_LIST(CL_OPTION_PRINT_ARGS)
		);
	exit(exitCode);
}

Options ParseCommandLineOptions(int argc, char const **argv)
{
	Options options = {0};

	options.program = (--argc, *argv++);

	bool needsMoreArguments = true;

	const char *inputFilePath = NULL;

	for (;;)
	{
		if (argc < 1)
		{
			if (needsMoreArguments) ExitPrintUsage(options.program, 1);
			else return options;
		}

		const char *argRest = NULL;
		enum OptionFlags flag = 0;

#define CL_OPTION_STRCMP(optionStr, arg0, optionNum, description) \
		if (strncmp((optionStr), *argv, sizeof(optionStr)-1) == 0) \
		{\
			argRest = *argv + sizeof(optionStr)-1;\
			flag = CL_OPTION_##optionNum;\
		} else
		CL_OPTION_LIST(CL_OPTION_STRCMP)
		{
			inputFilePath = *argv;
			break;
		}

		options.flags |= flag;

		if (flag == CL_OPTION_INPUT_DIRECT)
		{
			options.input.direct = argRest;
			needsMoreArguments = false;
		}

		--argc;
		++argv;
	}

	if (options.flags & CL_OPTION_INPUT_DIRECT)
	{
		return options;
	}
	else if (inputFilePath == NULL)
	{
		ExitPrintUsage(options.program, 1);
	}

	if (strcmp("-", inputFilePath) == 0)
	{
		options.input.file = stdin;
	}
	else
	{
		options.input.file = fopen(inputFilePath, "rb");
		if (options.input.file == NULL)
		{
			fprintf(stderr, "[ERROR] Could not open file, '%.256s'.\n", inputFilePath);
			exit(-1);
		}
	}

	return options;
}

int ReadEntireFile(FILE *file, char **contents, size_t *contentsLen)
{
	*contents = NULL;
	if (!file) return -1;
	if (fseek(file, 0, SEEK_END) < 0) goto error;
#ifndef _WIN32
    *contentsLen = ftell(file);
#else
    *contentsLen = _ftelli64(file);
#endif
    if (*contentsLen < 0) goto error;
    if (fseek(file, 0, SEEK_SET) < 0) goto error;

	*contents = malloc(*contentsLen + 1);
	if (1 != fread(*contents, *contentsLen, 1, file))
	{
		perror("Could not read file");
		goto error;
	}

	(*contents)[*contentsLen] = '\0';
	return 0;
error:
	fprintf(stderr, "[ERROR] Could not read file: %s\n", strerror(errno));
	if (file != stdin) fclose(file);
	if (*contents) free(*contents);
	return -1;
}

int main(int argc, char const *argv[])
{
	Options options = ParseCommandLineOptions(argc, argv);

	const char *input;

	if (options.flags & CL_OPTION_INPUT_DIRECT)
	{
		input = options.input.direct;
	}
	else
	{
		size_t inputContentsLen;
		if (ReadEntireFile(options.input.file, (char **)&input, &inputContentsLen) == -1)
		{
			return -1;
		}
	}

	TokenStream ts = TokenStreamFromCStr(input);

	Expr *parsedExpression = ParseExpression(&ts, 0, (Token){TOK_END_OF_STREAM});

	if (parsedExpression && parsedExpression->type == EXPR_PARSE_ERROR)
	{
		ParseError err = parsedExpression->as.error;
		fprintf(stderr, "Error parsing [location:%d:%d]: (%s)\n", err.line, err.column, err.message);
		return 1;
	}

	if (options.flags & CL_OPTION_PRINT_INFIX)
	{
		printf("Interpretation (Infix): ");
		PrintExprInfix(parsedExpression);
		printf("\n");
	}

	if (options.flags & CL_OPTION_PRINT_S)
	{
		printf("Interpretation (S-expression): ");
		PrintExprS(parsedExpression);
		printf("\n");
	}

	if (options.flags & CL_OPTION_PRINT_RPN)
	{
		printf("Interpretation (RPN): ");
		PrintExprRpn(parsedExpression);
		printf("\n");
	}

	if (parsedExpression)
	{
		double result = EvalExpr(parsedExpression);
		printf("%g\n", result);
	}
	else
	{
		printf("()\n");
	}

	return 0;
}
