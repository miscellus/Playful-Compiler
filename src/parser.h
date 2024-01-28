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

typedef struct BinNode_t BinNode;
typedef union Expr_t Expr;
typedef struct ParseError_t ParseError;


typedef enum
{
	OP_ADD = '+',
	OP_MINUS = '-',
	OP_MULTIPLY = '*',
	OP_DIVIDE = '/',
	OP_EXP = '^',
} Operator;

struct BinNode_t
{
	Operator op;
	Expr *lhs;
	Expr *rhs;
};

struct ParseError_t
{
	const char *message;
	int line;
	int column;
};

typedef enum
{
	EXPR_NUMBER,
	EXPR_BINOP,
	EXPR_PARSE_ERROR,
} ExprType;

union Expr_t
{
	ExprType type;

	struct {
		ExprType type;
		double v;
	} number;

	struct {
		ExprType type;
		BinNode v;
	} binop;

	struct {
		ExprType type;
		ParseError v;
	} error;
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