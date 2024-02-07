#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "tokenizer.h"

static void
OperatorPrecedence(int op, int *lPrec, int *rPrec)
{
	int prec = 0;
	int rightAssoc = 0;

#undef X
#define X(TOK, PREC, SAME_PREC, ASSOC) \
	case (TOK): \
		prec = (PREC); \
		rightAssoc = (ASSOC); \
		break;

	switch (op) {
		OP_PREC_LIST()
	}

	*lPrec = 2 * prec + (1 & rightAssoc);
	*rPrec = 2 * prec + (1 & (1 - rightAssoc));
}

static Expr *
ErrorExpr(int lineNumber, int characterColumn, const char *restrict messageFormat, ...)
{
	char messageBuffer[512];

	va_list args;
	va_start(args, messageFormat);
	int messageLen = vsnprintf(messageBuffer, sizeof(messageBuffer), messageFormat, args);
	va_end(args);

	assert(messageLen >= 0 && messageLen < (int)sizeof(messageBuffer));

	char *message = malloc(messageLen + 1);
	strncpy(message, messageBuffer, messageLen);

	Expr *result = malloc(sizeof(*result));
	result->h.type = EXPR_ERROR;
	result->u.error = (ParseError){
	    .message = message,
	    .line = lineNumber,
	    .column = characterColumn,
	};
	return result;
}

Expr *
ParseExpression(TokenStream *ts)
{
	return ParseSubExpression(ts, 0, (Token){TOK_INPUT_END});
}

Expr *
ParseSubExpression(TokenStream *ts, int minimumPrecedence, Token stopToken)
{
	bool negate = false;
	Token token;

	//
	// Parse LValue
	//
restart:
	token = NextToken(ts);
	Expr *lhs;

	if (token.h.type == TOK_IDENT) {
		lhs = malloc(sizeof(*lhs));
		lhs->h.type = EXPR_VARIABLE;
		// lhs->u.identifier =
	}
	else if (token.h.type == TOK_NUMBER) {
		lhs = malloc(sizeof(*lhs));
		lhs->h.type = EXPR_LITERAL_FLOAT64;
		lhs->u.number = token.u.number;
	}
	else if (token.h.type == '(') {
		lhs = ParseSubExpression(ts, 0, (Token){.h.type = ')'});

		Token endParen = NextToken(ts);
		if (endParen.h.type != ')') {
			return ErrorExpr(endParen.h.line, endParen.h.column, "Expected token ')', found '%c'", endParen.h.type);
		}
	}
	else if (token.h.type == '-') // Unary minus
	{
		negate = !negate;
		goto restart;
	}
	else if (token.h.type == TOK_INPUT_END) {
		Expr *unit = malloc(sizeof(*unit));
		unit->h.type = EXPR_UNIT;
		return unit;
	}
	else {
		return ErrorExpr(token.h.line, token.h.column, "Unexpected token, '%c'", token.h.type);
	}

	if (negate) {
		// XOR to toggle the negation of the left hand side expressoin
		lhs->h.flags ^= EXPR_FLAG_NEGATED;
	}

	//
	// Parse RValue
	//
	for (;;) {
		TokenStream tsTemp = *ts;
		Token tokOp = NextToken(&tsTemp);

		if (tokOp.h.type == stopToken.h.type) return lhs;

		switch (tokOp.h.type) {
			case '=': {
				if (lhs->h.type != EXPR_VARIABLE) {
					return ErrorExpr(tokOp.h.line, tokOp.h.column, "Left-hand side of operator '=' must be a variable");
				}
			} break;

			case '+':
			case '-':
			case '*':
			case '/':
			case '^': break;

			default:
				if (tokOp.h.type == TOK_IDENT) {
					return ErrorExpr(
					    tokOp.h.line,
					    tokOp.h.column,
					    "Unexpected identifier, '%s'",
					    tokOp.u.ident); // TODO(jkk): len-string
				}
				else {
					return ErrorExpr(tokOp.h.line, tokOp.h.column, "Unexpected token, '%c'", tokOp.h.type);
				}
		}

		int lPrec, rPrec;
		OperatorPrecedence(tokOp.h.type, &lPrec, &rPrec);

		if (lPrec < minimumPrecedence) {
			break;
		}

		ts->at = tsTemp.at;
		Expr *rhs = ParseSubExpression(ts, rPrec, stopToken);

		if (rhs->h.type == EXPR_UNIT) {
			return ErrorExpr(ts->lineCount, GetColumn(ts), "Operator '%c' missing right hand operand", tokOp.h.type);
		}
		else if (rhs->h.type == EXPR_ERROR) {
			return rhs;
		}

		Expr *newLhs = malloc(sizeof(*newLhs));
		newLhs->h.type = EXPR_BINOP;
		newLhs->u.binop = (BinNode){
		    .op = (Operator)tokOp.h.type,
		    .lhs = lhs,
		    .rhs = rhs,
		};

		lhs = newLhs;
	}

	return lhs;
}
