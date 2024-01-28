#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tokenizer.h"
#include "parser.h"

void PrintExprInfix(Expr *expr)
{
	switch (expr->type) {
	case EXPR_NUMBER:
		printf("%g", expr->number.v);
		break;

	case EXPR_BINOP:
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
	switch (expr->type) {
	case EXPR_NUMBER:
		printf("%g", expr->number.v);
		break;

	case EXPR_BINOP:
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
	switch (expr->type) {
	case EXPR_NUMBER:
		printf("%g", expr->number.v);
		break;

	case EXPR_BINOP:
		printf("(%c ", expr->binop.v.op);
		PrintExprS(expr->binop.v.lhs);
		printf(" ");
		PrintExprS(expr->binop.v.rhs);
		printf(")");
		break;

	case EXPR_PARSE_ERROR:
		assert(!"TODO: print parse error");
		break;
	}
}

double EvalExpr(Expr *expr)
{
	switch (expr->type)
	{
		case EXPR_NUMBER:
			return expr->number.v;

		case EXPR_BINOP:
		{
			BinNode bn = expr->binop.v;
			double lresult = EvalExpr(bn.lhs);
			double rresult = EvalExpr(bn.rhs);
			switch (bn.op)
			{
				case '+': return lresult + rresult;
				case '-': return lresult - rresult;
				case '*': return lresult * rresult;
				case '/': return lresult / rresult;
				case '^': return pow(lresult, rresult);
				default:
					return 42.0;
			}
		} break;

		case EXPR_PARSE_ERROR:
			assert(!"TODO: eval parse error");
			break;
	}
	return 1337.0;
}

static void ExitPrintUsage(const char *program, int exitCode)
{
	fprintf(stderr, "Missing argument.\n");
	fprintf(stderr, "USAGE: %s <expressions>\n", program);
	exit(exitCode);
}


int main(int argc, char const *argv[])
{
	const char *program = (--argc, *argv++);

	if (argc < 1)
	{
		ExitPrintUsage(program, 1);
	}

	enum OutputOpt
	{
		OUTPUT_JUST_RESULT = 0,
		OUTPUT_INFIX_PARENS = 1,
		OUTPUT_S_EXPR = 2,
		OUTPUT_RPN = 4,
	};

	enum OutputOpt outputOpt = OUTPUT_JUST_RESULT;

	for (;;)
	{
		if (strcmp("-print-infix", argv[0]) == 0)
		{
			--argc;
			++argv;
			outputOpt |= OUTPUT_INFIX_PARENS;
		}
		else if (strcmp("-print-s", argv[0]) == 0)
		{
			--argc;
			++argv;
			outputOpt |= OUTPUT_S_EXPR;
		}
		else if (strcmp("-print-rpn", argv[0]) == 0)
		{
			--argc;
			++argv;
			outputOpt |= OUTPUT_RPN;
		}
		else if (argc < 1)
		{
			ExitPrintUsage(program, 1);
		}
		else
		{
			break;
		}
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

	if (outputOpt & OUTPUT_INFIX_PARENS)
	{
		printf("Interpretation (Infix): ");
		PrintExprInfix(parsedExpression);
		printf("\n");
	}

	if (outputOpt & OUTPUT_S_EXPR)
	{
		printf("Interpretation (S-expression): ");
		PrintExprS(parsedExpression);
		printf("\n");
	}

	if (outputOpt & OUTPUT_RPN)
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
