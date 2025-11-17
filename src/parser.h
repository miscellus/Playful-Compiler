#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>
#include "tokenizer.h"

enum
{
	OP_ADD = '+',
	OP_MINUS = '-',
	OP_MULTIPLY = '*',
	OP_DIVIDE = '/',
	OP_EXP = '^',
};

typedef struct BinNode_t
{
	int op;
	struct Expr_t *lhs;
	struct Expr_t *rhs;
} BinNode;

typedef struct ParseError_t
{
	const char *message;
	int line;
	int column;
} ParseError;

typedef struct VariableExpr {
	Ident ident;
} VariableExpr;

typedef enum
{
	EXPR_NUMBER,
	EXPR_BINOP,
	EXPR_PARSE_ERROR,
	EXPR_VARIABLE,
} ExprType;

typedef enum
{
	EXPR_FLAG_NEGATED = (1 << 0),
} ExprFlags;

typedef struct Expr_t
{
	ExprType type;
	ExprFlags flags;
	struct Expr_t *next;

	union
	{
		double number;
		BinNode binop;
		ParseError error;
		VariableExpr variable;
	} as;
} Expr;

Expr *ParseExpression(TokenStream *ts, int minPrec, TokenType stopToken);

double EvalExpr(Expr *expr);

void PrintExprInfix(Expr *expr);
void PrintExprRpn(Expr *expr);
void PrintExprS(Expr *expr);

#endif