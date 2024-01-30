#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tokenizer.h"
#include "parser.h"

#define CL_OPTION_LIST(X) \
	X("-print-infix" , CL_OPTION_PRINT_INFIX , "Print parenthesized expression with infix operators.") \
	X("-print-s"     , CL_OPTION_PRINT_S     , "Print parenthesized s-expression.") \
	X("-print-rpn"   , CL_OPTION_PRINT_RPN   , "Print expression in reverse polish notation (RPN).") \
	//END

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
#define CL_OPTION_PRINT_FORMAT(optionStr, optionNum, description) "  %-18s %s\n"
#define CL_OPTION_PRINT_ARGS(optionStr, optionNum, description) ,optionStr, description

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

#define CL_OPTION_ENUM(optionStr, optionNum, description) optionNum,
enum ClOptionType
{
	CL_OPTION_NONE = 0,
	CL_OPTION_LIST(CL_OPTION_ENUM)
};

int main(int argc, char const *argv[])
{
	const char *program = (--argc, *argv++);

	enum ClOptionType outputOpt = 0;

	for (;;)
	{
		if (argc < 1)
		{
			ExitPrintUsage(program, 1);
		}

		bool again = false;

#define CL_OPTION_STRCMP(optionStr, optionNum, description) \
		if (strcmp((optionStr), argv[0]) == 0) \
		{ \
			outputOpt |= (1UL << (optionNum)); \
			again = true; \
		}

		CL_OPTION_LIST(CL_OPTION_STRCMP)

		if (!again) break;

		--argc;
		++argv;
	}

	unsigned long exprLen = 0;

	for (int i = 0; i < argc; ++i) {
		exprLen += strlen(argv[i]);
	}

	char *const exprBuf = calloc(1, exprLen + 1);

	char *at = exprBuf;
	for (int i = 0; i < argc; ++i) {
		if (i > 0)
		{
			*at++ = ' ';
		}
		long len = strlen(argv[i]);
		memcpy(at, argv[i], len);
		at += len;
	}
	*at = '\0';

	TokenStream ts = TokenStreamFromCStr(exprBuf);

	Expr *parsedExpression = ParseExpression(&ts, 0, (Token){TOK_END_OF_STREAM});

	if (parsedExpression && parsedExpression->h.type == EXPR_PARSE_ERROR)
	{
		ParseError err = parsedExpression->error.v;
		fprintf(stderr, "Error parsing [location:%d:%d]: (%s)\n", err.line, err.column, err.message);
		return 1;
	}

	if (outputOpt & (1UL << CL_OPTION_PRINT_INFIX))
	{
		printf("Interpretation (Infix): ");
		PrintExprInfix(parsedExpression);
		printf("\n");
	}

	if (outputOpt & (1UL << CL_OPTION_PRINT_S))
	{
		printf("Interpretation (S-expression): ");
		PrintExprS(parsedExpression);
		printf("\n");
	}

	if (outputOpt & (1UL << CL_OPTION_PRINT_RPN))
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
