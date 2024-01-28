#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "parser.h"
#include "tokenizer.h"

static void OperatorPrecedence(int op, int *lPrec, int *rPrec)
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

Expr *ParseExpression(TokenStream *ts, int minimumPrecedence, Token stopToken)
{
	Token token = NextToken(ts);

	Expr *lhs;

	if (token.type == TOK_NUMBER)
	{
		lhs = malloc(sizeof(*lhs));
		lhs->type = EXPR_NUMBER;
		lhs->number.v = token.number;
	}
	else if (token.type == '(')
	{
		lhs = ParseExpression(ts, 0, (Token){.type = ')'});
		NextToken(ts);
	}
	else
	{
		if (token.type != TOK_END_OF_STREAM)
		{
			fprintf(stderr, "Unexpected token: %d '%c'\n", token.type, token.type);
			fprintf(stderr, "At: '%s'\n", ts->at);
			assert(!"TODO: Error reporting (unknown) lhs token");
		}
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

		if (lPrec < minimumPrecedence)
		{
			break;
		}

		ts->at = tsTemp.at;
		Expr *rhs = ParseExpression(ts, rPrec, stopToken);

		Expr *newLhs = malloc(sizeof(*newLhs));
		newLhs->type = EXPR_BINOP;
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
