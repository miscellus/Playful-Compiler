#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tokenizer.h"
#include "parser.h"
#include "options.h"

void PrintExprInfix(Expr *expr)
{
	if (!expr) return;

	bool negated = false;
	if (expr->h.flags & EXPR_FLAG_NEGATED) negated = true;

	switch (expr->h.type)
	{
		case EXPR_UNIT:
		{
			printf("()");
		} break;

		case EXPR_NUMBER:
		{
			if (negated) printf("-");
			printf("%g", expr->number.v);
		} break;

		case EXPR_BINOP:
		{
			if (negated) printf("-");
			printf("(");
			PrintExprInfix(expr->binop.v.lhs);
			printf(" %c ", expr->binop.v.op);
			PrintExprInfix(expr->binop.v.rhs);
			printf(")");
		} break;

		case EXPR_ERROR:
		{
			assert(!"TODO: print parse error");
		} break;
	}
}

void PrintExprRPN(Expr *expr)
{
	bool negated = false;
	if (expr->h.flags & EXPR_FLAG_NEGATED) negated = true;

	switch (expr->h.type)
	{
		case EXPR_UNIT:
		{
			printf("()");
		} break;

		case EXPR_NUMBER:
		{
			if (negated) printf("-");
			printf("%g", expr->number.v);
		} break;

		case EXPR_BINOP:
		{
			if (negated) printf("-");
			PrintExprRPN(expr->binop.v.lhs);
			printf(" ");
			PrintExprRPN(expr->binop.v.rhs);
			printf(" %c", expr->binop.v.op);
		} break;

		case EXPR_ERROR:
		{
			assert(!"TODO: print parse error");
		} break;
	}
}

void PrintExprS(Expr *expr)
{
	bool negated = false;
	if (expr->h.flags & EXPR_FLAG_NEGATED) negated = true;

	switch (expr->h.type)
	{
		case EXPR_UNIT:
		{
			printf("()");
		} break;

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

		case EXPR_ERROR:
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
		case EXPR_UNIT:
		{
			assert(!"The unit expression does not have a numeric value.");
		} break;

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

		case EXPR_ERROR:
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

	Expr *expr = ParseExpression(&ts);
	assert(expr);

	if (expr && expr->h.type == EXPR_ERROR)
	{
		ParseError err = expr->error.v;
		fprintf(stderr, "Error parsing [location:%d:%d]: (%s)\n", err.line, err.column, err.message);
		return 1;
	}

	if (options.flags & CL_OPTION_PRINT_INFIX)
	{
		printf("Interpretation (Infix): ");
		PrintExprInfix(expr);
		printf("\n");
	}

	if (options.flags & CL_OPTION_PRINT_S)
	{
		printf("Interpretation (S-expression): ");
		PrintExprS(expr);
		printf("\n");
	}

	if (options.flags & CL_OPTION_PRINT_RPN)
	{
		printf("Interpretation (RPN): ");
		PrintExprRPN(expr);
		printf("\n");
	}

	if (expr->h.type != EXPR_UNIT)
	{
		double result = EvalExpr(expr);
		printf("%g\n", result);
	}
	else
	{
		printf("()\n");
	}

	return 0;
}
