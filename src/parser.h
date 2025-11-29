#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>
#include "tokenizer.h"
#include "var_table.h"

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
	EXPR_SEQUENCE,
} ExprType;

typedef enum
{
	EXPR_FLAG_NEGATED = (1 << 0),
} ExprFlags;

typedef struct ExprSeq_t
{
	struct Expr_t *expr;
	struct ExprSeq_t *next;
} ExprSeq;

typedef struct Expr_t
{
	ExprType type;
	ExprFlags flags;

	union
	{
		double number;
		BinNode binop;
		ParseError error;
		VariableExpr variable;
		ExprSeq seq;
	} as;
} Expr;


Expr *ParseExprSeq(TokenStream *ts);
Expr *ParseExpr(TokenStream *ts, int minPrec, TokenType stopToken);

double EvalExpr(VarTable *vars, Expr *expr);

void PrintExpr(Expr *expr);
void PrintExprS(Expr *expr);

#endif