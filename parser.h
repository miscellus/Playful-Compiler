#ifndef PARSER_H
#define PARSER_H

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

Expr *ParseExpression(TokenStream *ts, int minPrec, Token stopToken);

#endif