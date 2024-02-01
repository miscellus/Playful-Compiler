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

void PrintExprInfix(Expr *expr)
{
	if (!expr) return;

	bool negated = false;
	if (expr->h.flags & EXPR_FLAG_NEGATED) negated = true;

	switch (expr->h.type) {
	case EXPR_NUMBER:
		if (negated) printf("-");
		printf("%g", expr->number.v);
		break;

	case EXPR_BINOP:
		if (negated) printf("-");
		printf("(");
		PrintExprInfix(expr->binop.v.lhs);
		printf(" %c ", expr->binop.v.op);
		PrintExprInfix(expr->binop.v.rhs);
		printf(")");
		break;

	case EXPR_PARSE_ERROR:
		assert(!"TODO: print parse error");
		break;
	}
}

void PrintExprRPN(Expr *expr)
{
	bool negated = false;
	if (expr->h.flags & EXPR_FLAG_NEGATED) negated = true;

	switch (expr->h.type) {
	case EXPR_NUMBER:
		if (negated) printf("-");
		printf("%g", expr->number.v);
		break;

	case EXPR_BINOP:
		if (negated) printf("-");
		PrintExprRPN(expr->binop.v.lhs);
		printf(" ");
		PrintExprRPN(expr->binop.v.rhs);
		printf(" %c", expr->binop.v.op);
		break;

	case EXPR_PARSE_ERROR:
		assert(!"TODO: print parse error");
		break;
	}
}

void PrintExprS(Expr *expr)
{
	bool negated = false;
	if (expr->h.flags & EXPR_FLAG_NEGATED) negated = true;

	switch (expr->h.type)
	{
		case EXPR_NUMBER:
		{
			if (negated) printf("-");
			printf("%g", expr->number.v);
		} break;

		case EXPR_BINOP:
		{
			printf("(%c ", expr->binop.v.op);
			if (negated) printf("-");
			PrintExprS(expr->binop.v.lhs);
			printf(" ");
			PrintExprS(expr->binop.v.rhs);
			printf(")");
		} break;

		case EXPR_PARSE_ERROR:
		{
			assert(!"TODO: print parse error");
		} break;
	}
}

double EvalExpr(Expr *expr)
{
	double result = 0;

	switch (expr->h.type)
	{
		case EXPR_NUMBER:
		{
			result = expr->number.v;
		} break;

		case EXPR_BINOP:
		{
			BinNode bn = expr->binop.v;
			double lresult = EvalExpr(bn.lhs);
			double rresult = EvalExpr(bn.rhs);
			switch (bn.op)
			{
				case '+': result = lresult + rresult; break;
				case '-': result = lresult - rresult; break;
				case '*': result = lresult * rresult; break;
				case '/': result = lresult / rresult; break;
				case '^': result = pow(lresult, rresult); break;
				default:
					return 42.0;
			}
		} break;

		case EXPR_PARSE_ERROR:
		{
			assert(!"TODO: eval parse error");
		} break;
	}

	if (expr->h.flags & EXPR_FLAG_NEGATED)
	{
		result = -result;
	}

	return result;
}

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

	if (argc < 1) ExitPrintUsage(options.program, 1);

	const char *inputFile = (--argc, *argv++);

	if (strcmp("-", inputFile) == 0)
	{
		assert(!"TODO");
		options.input.file = stdin;
	}
	else
	{
		options.input.file = fopen(inputFile, "r");
		if (options.input.file == NULL)
		{
			fprintf(stderr, "Could not open file.\n");
			ExitPrintUsage(options.program, 1);
		}
	}

	return options;
}

int ReadEntireFile(FILE *file, char **contents, long *contentsLen)
{
	fseek(file, 0, SEEK_END);
	*contentsLen = ftell(file);
	fseek(file, 0, SEEK_SET);

	*contents = malloc((*contentsLen + 1) * sizeof(**contents));

	if (1 != fread(*contents, *contentsLen, 1, file))
	{
		fclose(file);
		return -1;
	}

	(*contents)[*contentsLen] = '\0';
	return 0;
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
		long inputContentsLen;
		if (ReadEntireFile(options.input.file, (char **)&input, &inputContentsLen) == -1)
		{
			ExitPrintUsage(options.program, 1);
		}
	}

	TokenStream ts = TokenStreamFromCStr(input);

	Expr *parsedExpression = ParseExpression(&ts, 0, (Token){TOK_END_OF_STREAM});

	if (parsedExpression && parsedExpression->h.type == EXPR_PARSE_ERROR)
	{
		ParseError err = parsedExpression->error.v;
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
		PrintExprRPN(parsedExpression);
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
