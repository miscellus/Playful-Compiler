#ifndef PARSER_H
#define PARSER_H

#include "tokenizer.h"

enum OpAssociativity {
	LEFT_TO_RIGHT = 0,
	RIGHT_TO_LEFT = 1,
};

#define OP_PREC_LIST() \
	X('=', PREC_ASSIGN, , RIGHT_TO_LEFT) \
	X('+', PREC_ADD, , LEFT_TO_RIGHT) \
	X('-', PREC_MINUS, = PREC_ADD, LEFT_TO_RIGHT) \
	X('*', PREC_TIMES, , LEFT_TO_RIGHT) \
	X('/', PREC_DIVIDE, = PREC_TIMES, LEFT_TO_RIGHT) \
	X('^', PREC_EXP, , RIGHT_TO_LEFT) \
	// END

typedef struct Expr Expr;

typedef enum {
	OP_ASSIGN = '=',
	OP_ADD = '+',
	OP_SUBTRACT = '-',
	OP_MULTIPLY = '*',
	OP_DIVIDE = '/',
	OP_EXPONENT = '^',
} Operator;

typedef struct BinNode {
	Operator op;
	Expr *lhs;
	Expr *rhs;
} BinNode;

typedef struct ParseError {
	const char *message;
	int line;
	int column;
} ParseError;

typedef enum {
	EXPR_UNIT,
	EXPR_LITERAL_FLOAT64,
	EXPR_VARIABLE,
	EXPR_BINOP,
	EXPR_ERROR,
} ExprType;

typedef enum {
	EXPR_FLAG_NEGATED = (1 << 0),
} ExprFlags;

typedef struct ExprHeader {
	ExprType type;
	ExprFlags flags;
} ExprHeader;

struct Expr {
	ExprHeader h;

	union {
		double number;
		BinNode binop;
		ParseError error;
	} u;
};

#undef X
#define X(TOK, PREC, SAME_PREC, ASSOC) PREC SAME_PREC,

enum OperatorPrecedence {
	PREC_ROOT,

	OP_PREC_LIST()
};

Expr *
ParseExpression(TokenStream *ts);

Expr *
ParseSubExpression(TokenStream *ts, int minimumPrecedence, Token stopToken);

#endif