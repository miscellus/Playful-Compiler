#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokenizer.h"

#define ASSOC_LEFT 0
#define ASSOC_RIGHT 1

#define OP_PREC_LIST() \
	X('+' , PREC_ADD    ,            , ASSOC_LEFT) \
	X('-' , PREC_MINUS  , = PREC_ADD , ASSOC_LEFT) \
	X('*' , PREC_TIMES  ,            , ASSOC_LEFT) \
	X('/' , PREC_DIVIDE ,            , ASSOC_LEFT) \
	X('^' , PREC_EXP    ,            , ASSOC_RIGHT) \
	// END

typedef struct BinNode_t BinNode;
typedef union Expr_t Expr;

typedef enum
{
	OP_ADD = '+',
	OP_MINUS = '-',
	OP_MULTIPLY = '*',
	OP_DIVIDE = '/',
	OP_EXP = '^',
	OP_FN_COMP = '.',
} Operator;

struct BinNode_t
{
	Operator op;
	Expr *lhs;
	Expr *rhs;
};

typedef enum
{
	EXPR_NUMBER,
	EXPR_BINOP,
} ExprType;

union Expr_t
{
	ExprType exprType;

	struct {
		ExprType exprType;
		double v;
	} number;

	struct {
		ExprType exprType;
		BinNode v;
	} binop;
};


#undef X
#define X(TOK, PREC, SAME_PREC, ASSOC) PREC SAME_PREC,

typedef enum
{
	PREC_ROOT,

	OP_PREC_LIST()
} OpPrec;

void OperatorPrecedence(int op, int *lPrec, int *rPrec)
{
	int prec = 0;
	int rightAssoc = 0;

#undef X
#define X(TOK, PREC, SAME_PREC, ASSOC) \
	case (TOK): prec = (PREC); rightAssoc = (ASSOC); break;

	switch (op)
	{
		OP_PREC_LIST()
	}

	*lPrec = 2*prec + (1 & rightAssoc);
	*rPrec = 2*prec + (1 & (1 - rightAssoc));
}

Expr *ParseExpression(TokenStream *ts, int minPrec, Token stopToken)
{
	Token tok = NextToken(ts);

	Expr *lhs;

	if (tok.type == TOK_NUMBER)
	{
		lhs = malloc(sizeof(*lhs));
		lhs->exprType = EXPR_NUMBER;
		lhs->number.v = tok.number;
	}
	else if (tok.type == '(')
	{
		lhs = ParseExpression(ts, 0, (Token){.type = ')'});
		NextToken(ts);
	}
	else
	{
		fprintf(stderr, "Unexpected token: %d '%c'\n", tok.type, tok.type);
		fprintf(stderr, "At: '%s'\n", ts->at);
		assert(!"TODO: Error reporting (unknown) lhs token");
		return NULL;
	}

	for (;;)
	{
		TokenStream tsTemp = *ts;
		Token tokOp = NextToken(&tsTemp);

		if (tokOp.type == stopToken.type)
			return lhs;

		switch (tokOp.type)
		{
		case '+':
		case '-':
		case '*':
		case '/':
		case '^':
			break;

		default:
			assert(!"TODO: Error reporting (unknown) token");
			return NULL; // TODO(jkk): error reporting
		}

		int lPrec, rPrec;
		OperatorPrecedence(tokOp.type, &lPrec, &rPrec);

		if (lPrec < minPrec)
		{
			break;
		}

		ts->at = tsTemp.at;
		Expr *rhs = ParseExpression(ts, rPrec, stopToken);

		Expr *newLhs = malloc(sizeof(*newLhs));
		newLhs->exprType = EXPR_BINOP;
		newLhs->binop.v = (BinNode)
		{
			.op = tokOp.type,
			.lhs = lhs,
			.rhs = rhs,
		};

		lhs = newLhs;
	}

	return lhs;
}

void PrintExprInfix(Expr *expr)
{
	switch (expr->exprType) {
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
	}
}

void PrintExprRPN(Expr *expr)
{
	switch (expr->exprType) {
	case EXPR_NUMBER:
		printf("%g", expr->number.v);
		break;

	case EXPR_BINOP:
		PrintExprRPN(expr->binop.v.lhs);
		printf(" ");
		PrintExprRPN(expr->binop.v.rhs);
		printf(" %c", expr->binop.v.op);
		break;
	}
}

void PrintExprS(Expr *expr)
{
	switch (expr->exprType) {
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
	}
}

double EvalExpr(Expr *expr)
{
	switch (expr->exprType)
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
	}
	return 1337.0;
}


int main(int argc, char const *argv[])
{
	const char *program = (--argc, *argv++);

	if (argc < 1)
	{
		fprintf(stderr, "Missing argument.\n");
		fprintf(stderr, "USAGE: %s <expressions>\n", program);
		return 1;
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
			fprintf(stderr, "Missing argument.\n");
			fprintf(stderr, "USAGE: %s <expressions>\n", program);
			return 1;
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
		long len = strlen(argv[i]);
		memcpy(at, argv[i], len);
		at += len;
		*at++ = ' ';
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

	double result = EvalExpr(parsedExpression);
	printf("%g\n", result);

	return 0;
}