#include <math.h>
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
	int p = 0; // Precedence
	int r = 0; // Right associate

	switch (op)
	{
		case '+': p = 0x100; break;
		case '-': p = 0x100; break;
		case '*': p = 0x200; break;
		case '/': p = 0x200; break;
		case '^': p = 0x300; r = 1; break;
		default:
			assert(0 && "Invalid code path!");
	}

	*lPrec = 2*p + (1 & r);
	*rPrec = 2*p + (1 & (1 - r));
}

static Expr *ErrorExpr(int lineNumber, int characterColumn, const char *restrict messageFormat, ...)
{
	char messageBuffer[512];

	va_list args;
	va_start(args, messageFormat);
	int messageLen = vsnprintf(messageBuffer, sizeof(messageBuffer), messageFormat, args);
	va_end(args);

	assert(messageLen >= 0 && messageLen < (int)sizeof(messageBuffer));

	char *message = malloc(messageLen + 1);
	strncpy(message, messageBuffer, messageLen);

	Expr *result = calloc(1, sizeof(*result));
	result->type = EXPR_PARSE_ERROR;
	result->as.error = (ParseError){
	    .message = message,
	    .line = lineNumber,
	    .column = characterColumn,
	};
	return result;
}

Expr *ParseExpression(TokenStream *ts, int minimumPrecedence, Token stopToken)
{
	bool negate = false;
	Token token;

	//
	// Parse LValue
	//
restart:
 	token = NextToken(ts);
	Expr *lhs;

	if (token.type == TOK_IDENT) {
		lhs = calloc(1, sizeof(*lhs));
		lhs->type = EXPR_VARIABLE;
		lhs->as.variable = (VariableExpr){.ident = token.as.ident};
	}
	else if (token.type == TOK_NUMBER)
	{
		lhs = calloc(1, sizeof(*lhs));
		lhs->type = EXPR_NUMBER;
		lhs->as.number = token.as.number;
	}
	else if (token.type == '(')
	{
		lhs = ParseExpression(ts, 0, (Token){.type = ')'});

		Token endParen = NextToken(ts);
		if (endParen.type != ')')
		{
			return ErrorExpr(
				endParen.line, endParen.column,
				"Expected token ')', found: %d '%c'",
				endParen.type, endParen.type);
		}
	}
	else if (token.type == '-') // Unary minus
	{
		negate = !negate;
		goto restart;
	}
	else if (token.type == TOK_INPUT_END)
	{
		return NULL;
	}
	else
	{
		return ErrorExpr(
			token.line, token.column,
			"Unexpected token: %d '%c'",
			token.type, token.type);
	}

	if (negate)
	{
		// XOR to toggle the negation of the left hand side expressoin
		lhs->flags ^= EXPR_FLAG_NEGATED;
	}

	//
	// Parse RValue
	//
	for (;;)
	{
		TokenStream tsTemp = *ts;
		Token tokOp = NextToken(&tsTemp);

		if (tokOp.type == stopToken.type)
			return lhs;

		switch (tokOp.type)
		{
		case '=': {
			if (lhs->type != EXPR_VARIABLE) {
				return ErrorExpr(tokOp.line, tokOp.column, "Left-hand side of operator '=' must be a variable");
			}
		} break;

		case '+':
		case '-':
		case '*':
		case '/':
		case '^':
			break;

		default:
			if (tokOp.type == TOK_IDENT) {
				return ErrorExpr(
				    tokOp.line, tokOp.column,
				    "Unexpected identifier, '%.*s'",
				    tokOp.as.ident.len, tokOp.as.ident.chars);
			}
			else {
				return ErrorExpr(
					tokOp.line, tokOp.column,
					"Unexpected token: %d '%c'",
					tokOp.type, tokOp.type);
			}
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
				ts->lineCount, GetColumn(ts),
				"Operator '%c' missing right hand operand",
				tokOp.type);
		}
		else if (rhs->type == EXPR_PARSE_ERROR)
		{
			return rhs;
		}

		Expr *newLhs = calloc(1, sizeof(*newLhs));
		newLhs->type = EXPR_BINOP;
		newLhs->as.binop = (BinNode)
		{
			.op = tokOp.type,
			.lhs = lhs,
			.rhs = rhs,
		};

		lhs = newLhs;
	}

	return lhs;
}

double EvalExpr(Expr *expr)
{
	double result = 0;

	switch (expr->type)
	{
		case EXPR_NUMBER:
		{
			result = expr->as.number;
		} break;

		case EXPR_BINOP:
		{
			BinNode bn = expr->as.binop;
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

		default:
			assert(0 && "Invalid code path!");
	}

	if (expr->flags & EXPR_FLAG_NEGATED)
	{
		result = -result;
	}

	return result;
}

void PrintExprInfix(Expr *expr)
{
	if (!expr) return;

	bool negated = false;
	if (expr->flags & EXPR_FLAG_NEGATED) negated = true;

	switch (expr->type) {
	case EXPR_NUMBER:
		if (negated) printf("-");
		printf("%g", expr->as.number);
		break;

	case EXPR_BINOP:
		if (negated) printf("-");
		printf("(");
		PrintExprInfix(expr->as.binop.lhs);
		printf(" %c ", expr->as.binop.op);
		PrintExprInfix(expr->as.binop.rhs);
		printf(")");
		break;

	case EXPR_PARSE_ERROR:
		assert(!"TODO: print parse error");
		break;
	}
}

void PrintExprRpn(Expr *expr)
{
	bool negated = false;
	if (expr->flags & EXPR_FLAG_NEGATED) negated = true;

	switch (expr->type) {
	case EXPR_NUMBER:
		if (negated) printf("-");
		printf("%g", expr->as.number);
		break;

	case EXPR_BINOP:
		if (negated) printf("-");
		PrintExprRpn(expr->as.binop.lhs);
		printf(" ");
		PrintExprRpn(expr->as.binop.rhs);
		printf(" %c", expr->as.binop.op);
		break;

	case EXPR_PARSE_ERROR:
		assert(!"TODO: print parse error");
		break;
	}
}

void PrintExprS(Expr *expr)
{
	bool negated = false;
	if (expr->flags & EXPR_FLAG_NEGATED) negated = true;

	switch (expr->type)
	{
		case EXPR_NUMBER:
		{
			if (negated) printf("-");
			printf("%g", expr->as.number);
		} break;

		case EXPR_BINOP:
		{
			printf("(%c ", expr->as.binop.op);
			if (negated) printf("-");
			PrintExprS(expr->as.binop.lhs);
			printf(" ");
			PrintExprS(expr->as.binop.rhs);
			printf(")");
		} break;

		case EXPR_PARSE_ERROR:
		{
			assert(!"TODO: print parse error");
		} break;
	}
}