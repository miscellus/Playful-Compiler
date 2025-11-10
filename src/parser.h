#ifndef PARSER_H
#define PARSER_H

#include "tokenizer.h"

typedef enum
{
	OP_ADD = '+',
	OP_MINUS = '-',
	OP_MULTIPLY = '*',
	OP_DIVIDE = '/',
	OP_EXP = '^',
} Operator;

typedef struct Expr_t Expr;

typedef struct BinNode_t
{
	Operator op;
	Expr *lhs;
	Expr *rhs;
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

struct Expr_t
{
	ExprType type;
	ExprFlags flags;

	union
	{
		double number;
		BinNode binop;
		ParseError error;
		VariableExpr variable;
	} as;
};


Expr *ParseExpression(TokenStream *ts, int minPrec, Token stopToken);

double EvalExpr(Expr *expr);

void PrintExprInfix(Expr *expr);
void PrintExprRpn(Expr *expr);
void PrintExprS(Expr *expr);

#endif