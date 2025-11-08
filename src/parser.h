#ifndef PARSER_H
#define PARSER_H

#include "tokenizer.h"

#define ASSOC_LEFT 0
#define ASSOC_RIGHT 1

#define OP_PREC_LIST() \
	X('+' , PREC_ADD    ,              , ASSOC_LEFT) \
	X('-' , PREC_MINUS  , = PREC_ADD   , ASSOC_LEFT) \
	X('*' , PREC_TIMES  ,              , ASSOC_LEFT) \
	X('/' , PREC_DIVIDE , = PREC_TIMES , ASSOC_LEFT) \
	X('^' , PREC_EXP    ,              , ASSOC_RIGHT) \
	// END

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

typedef enum
{
	EXPR_NUMBER,
	EXPR_BINOP,
	EXPR_PARSE_ERROR,
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
	} as;
};


#undef X
#define X(TOK, PREC, SAME_PREC, ASSOC) PREC SAME_PREC,

typedef enum
{
	PREC_ROOT,

	OP_PREC_LIST()
} OpPrec;

Expr *ParseExpression(TokenStream *ts, int minPrec, Token stopToken);

#endif