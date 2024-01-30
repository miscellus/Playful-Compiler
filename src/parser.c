#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
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
	char messageBuffer[512];

	va_list args;
	va_start(args, column);
	int messageLen = vsnprintf(messageBuffer, sizeof(messageBuffer), messageFormat, args);
	va_end(args);

	assert(messageLen >= 0 && messageLen < (int)sizeof(messageBuffer));

	char *message = malloc(messageLen+1);
	strncpy(message, messageBuffer, messageLen);

	Expr *result = malloc(sizeof(*result));
	result->h.type = EXPR_PARSE_ERROR;
	result->error.v = (ParseError){
		.message = message,
		.line = line,
		.column = column,
	};
	return result;
}

Expr *ParseExpression(TokenStream *ts, int minimumPrecedence, Token stopToken)
{
	bool negate = false;
	Token token;

restart:
 	token = NextToken(ts);
	Expr *lhs;

	if (token.type == TOK_NUMBER)
	{
		lhs = malloc(sizeof(*lhs));
		lhs->h.type = EXPR_NUMBER;
		lhs->number.v = token.number;
	}
	else if (token.type == '(')
	{
		lhs = ParseExpression(ts, 0, (Token){.type = ')'});

		Token endParen = NextToken(ts);
		if (endParen.type != ')')
		{
			return ErrorExpr(
				"Expected token ')', found: %d '%c'",
				endParen.line, endParen.column,
				endParen.type, endParen.type);
		}
	}
	else if (token.type == '-') // Unary minus
	{
		negate = !negate;
		goto restart;
	}
	else if (token.type == TOK_END_OF_STREAM)
	{
		return NULL;
	}
	else
	{
		return ErrorExpr(
			"Unexpected token: %d '%c'",
			token.line, token.column,
			token.type, token.type);
	}

	if (negate)
	{
		// XOR to toggle the negation of the left hand side expressoin
		lhs->h.flags ^= EXPR_FLAG_NEGATED;
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
				"Unexpected token: %d '%c'",
				tokOp.line, tokOp.column,
				tokOp.type, tokOp.type);
		}

		int lPrec, rPrec;
		OperatorPrecedence(tokOp.type, &lPrec, &rPrec);

		if (lPrec < minimumPrecedence)
		{
			break;
		}

		ts->at = tsTemp.at;
		Expr *rhs = ParseExpression(ts, rPrec, stopToken);

		if (rhs == NULL)
		{
			return ErrorExpr(
				"Unexpected end of stream",
				ts->lineCount, GetColumn(ts),
				tokOp.type, tokOp.type);
		}
		else if (rhs->h.type == EXPR_PARSE_ERROR)
		{
			return rhs;
		}

		Expr *newLhs = malloc(sizeof(*newLhs));
		newLhs->h.type = EXPR_BINOP;
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
