#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


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

static Expr *ErrorExpr(const char *restrict messageFormat, int line, int column, ...)
{
	static char messageBuffer[512]; // @not-thread-safe

	va_list args;
	va_start(args, column);
	int messageLen = vsnprintf(messageBuffer, sizeof(messageBuffer), messageFormat, args);
	va_end(args);

	assert(messageLen >= 0 && messageLen < (int)sizeof(messageBuffer));

	char *message = malloc(messageLen+1);
	strncpy(message, messageBuffer, messageLen);

	Expr *result = malloc(sizeof(*result));
	result->type = EXPR_PARSE_ERROR;
	result->error.v = (ParseError){
		.message = message,
		.line = line,
		.column = column,
	};
	return result;
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
			return ErrorExpr(
				"Unexpected token: %d '%c'\n"
				"At: %s\n",
				token.line, token.column,
				token.type, token.type,
				ts->at);
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
			return ErrorExpr(
				"Unexpected token: %d '%c'\n"
				"At: %s\n",
				tokOp.line, tokOp.column,
				tokOp.type, tokOp.type,
				ts->at);
		}

		int lPrec, rPrec;
		OperatorPrecedence(tokOp.type, &lPrec, &rPrec);

		if (lPrec < minimumPrecedence)
		{
			break;
		}

		ts->at = tsTemp.at;
		Expr *rhs = ParseExpression(ts, rPrec, stopToken);

		if (rhs && rhs->type == EXPR_PARSE_ERROR)
		{
			return rhs;
		}

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
