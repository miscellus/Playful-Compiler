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
		case ';': p = 0x080; break;
		case '=': p = 0x100; r = 1; break;
		case '+': p = 0x200; break;
		case '-': p = 0x200; break;
		case '*': p = 0x300; break;
		case '/': p = 0x300; break;
		case '^': p = 0x400; r = 1; break;
		default:
			assert(0 && "Invalid code path!");
	}

	*lPrec = 2*p + (1 & r);
	*rPrec = 2*p + (1 & (1 - r));
}

static Expr *ErrorExpr(Parser *p, int lineNumber, int characterColumn, const char *restrict messageFormat, ...)
{
	char messageBuffer[512];

	va_list args;
	va_start(args, messageFormat);
	int messageLen = vsnprintf(messageBuffer, sizeof(messageBuffer), messageFormat, args);
	va_end(args);

	assert(messageLen >= 0 && messageLen < (int)sizeof(messageBuffer));

	char *message = msc_arena_push_array(&p->arena, char, messageLen + 1);
	strncpy(message, messageBuffer, messageLen);

	Expr *result = msc_arena_push(&p->arena, Expr);
	result->type = EXPR_PARSE_ERROR;
	result->as.error = (ParseError){
		.message = message,
		.line = lineNumber,
		.column = characterColumn,
	};
	return result;
}

Expr *ParseExprSeq(Parser *p)
{
	TokenStream *ts = p->ts;
	int prec;
	int ignore;
	OperatorPrecedence(';', &prec, &ignore);

	ExprSeq head = {0};
	ExprSeq *seq = &head;

	for (;;) {
		seq->expr = ParseExpr(p, prec, TOK_INPUT_END);
		if (seq->expr == NULL) break;

		TokenStream rewindPoint = *ts;
		Token tok = NextToken(ts);
		if (tok.type != ';')
		{
			*ts = rewindPoint;
			break;
		}

		seq = seq->next = msc_arena_push(&p->arena, ExprSeq);
	}

	Expr *expr = head.expr;

	if (head.next)
	{
		expr = msc_arena_push(&p->arena, Expr);
		expr->type = EXPR_SEQUENCE;
		expr->as.seq = head;
	}

	return expr;
}

Expr *ParseExpr(Parser *p, int minimumPrecedence, TokenType stopToken)
{
	TokenStream *ts = p->ts;
	bool negate = false;
	Token token = {0};

	//
	// Parse LValue
	//
restart:
	token = NextToken(ts);
	Expr *lhs = NULL;

	switch (token.type)
	{

	// Unary minus
	case '-':
		negate = !negate;
		goto restart;

	case TOK_IDENT:
		lhs = msc_arena_push(&p->arena, Expr);
		lhs->type = EXPR_VARIABLE;
		lhs->as.variable = (VariableExpr){.ident = token.as.ident};
	break;

	case TOK_NUMBER:
		lhs = msc_arena_push(&p->arena, Expr);
		lhs->type = EXPR_NUMBER;
		lhs->as.number = token.as.number;
	break;

	case '(':
	{
		lhs = ParseExprSeq(p);

		Token endParen = NextToken(ts);
		if (endParen.type != ')')
		{
			return ErrorExpr(
				p,
				endParen.line, endParen.column,
				"Expected token ')', found: %d '%c'",
				endParen.type, endParen.type);
		}
	}
	break;

	case TOK_INPUT_END:
	default:
		return NULL;

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

		switch (tokOp.type)
		{
		case '=': {
			if (lhs->type != EXPR_VARIABLE) {
				return ErrorExpr(p, tokOp.line, tokOp.column, "Left-hand side of operator '=' must be a variable");
			}
		} break;

		case '+':
		case '-':
		case '*':
		case '/':
		case '^':
			break;

		default:
			return lhs;
		}

		int lPrec, rPrec;
		OperatorPrecedence(tokOp.type, &lPrec, &rPrec);

		if (lPrec < minimumPrecedence)
		{
			break;
		}

		ts->at = tsTemp.at;

		Expr *rhs = ParseExpr(p, rPrec, stopToken);

		if (rhs == NULL)
		{
			return ErrorExpr(
				p,
				ts->lineCount, GetColumn(ts),
				"Operator '%c' missing right hand operand",
				tokOp.type);
		}
		else if (rhs->type == EXPR_PARSE_ERROR)
		{
			return rhs;
		}

		Expr *newLhs = msc_arena_push(&p->arena, Expr);
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

static double variables[256];

double EvalExpr(VarTable *vars, Expr *expr)
{
	double result = 0;

	switch (expr->type)
	{
		case EXPR_NUMBER:
			result = expr->as.number;
			break;

		case EXPR_VARIABLE:
		{
			VarEntry *e = vartable_find_ident(vars, expr->as.variable.ident);
			result = e ? e->val : 0.0;
		} break;

		case EXPR_BINOP:
		{
			BinNode bn = expr->as.binop;
			double rresult = EvalExpr(vars, bn.rhs);

			if (bn.op == '=')
			{
				assert(bn.lhs->type == EXPR_VARIABLE && "Left-hand of assignment must be variable");
				VarEntry *dest = vartable_get_or_create_ident(vars, bn.lhs->as.variable.ident);
				dest->val = rresult;
				result = rresult;
				break;
			}

			double lresult = EvalExpr(vars, bn.lhs);

			switch (bn.op)
			{
				case '+': result = lresult + rresult; break;
				case '-': result = lresult - rresult; break;
				case '*': result = lresult * rresult; break;
				case '/': result = lresult / rresult; break;
				case '^': result = pow(lresult, rresult); break;
				default:
					assert(!"TODO: unsupported operator");
			}
		} break;

		case EXPR_SEQUENCE:
		{
			ExprSeq *seq = &expr->as.seq;
			while (seq)
			{
				result = EvalExpr(vars, seq->expr);
				seq = seq->next;
			}
		} break;

		case EXPR_PARSE_ERROR:
			assert(!"TODO: eval parse error");
			break;

		default:
			assert(0 && "Invalid code path!");
	}

	if (expr->flags & EXPR_FLAG_NEGATED)
		result = -result;

	return result;
}


void PrintExpr(Expr *expr)
{
	if (!expr) return;

	bool negated = false;
	if (expr->flags & EXPR_FLAG_NEGATED) negated = true;

	switch (expr->type) {
	case EXPR_NUMBER:
		if (negated) printf("-");
		printf("%g", expr->as.number);
		break;

	case EXPR_VARIABLE:
		if (negated) printf("-");
		printf("%.*s", (int)expr->as.variable.ident.len, expr->as.variable.ident.chars);
		break;

	case EXPR_BINOP:
		if (negated) printf("-");
		printf("(");
		PrintExpr(expr->as.binop.lhs);
		printf(" %c ", expr->as.binop.op);
		PrintExpr(expr->as.binop.rhs);
		printf(")");
		break;


	case EXPR_SEQUENCE:
	{
		ExprSeq *seq = &expr->as.seq;
		for (;;)
		{
			PrintExpr(seq->expr);
			seq = seq->next;
			if (!seq) break;
			printf(" ; ");
		}
	} break;

	case EXPR_PARSE_ERROR:
		if (negated) printf("-");
		printf("<parse error: %s at %d:%d>", expr->as.error.message, expr->as.error.line, expr->as.error.column);
		break;

	default:
		assert(0 && "Invalid expr type");
	}
}


static void PrintIndent(int indent)
{
	for (int i = indent; i > 0 ; --i) putchar('\t');
}

static void PrintExprS_(const Expr *expr, int level)
{
	if (!expr) { PrintIndent(level); puts("()"); return; }

	bool neg = (expr->flags & EXPR_FLAG_NEGATED) != 0;

	switch (expr->type) {
	case EXPR_NUMBER:
		PrintIndent(level);
		if (neg) putchar('-');
		printf("%g\n", expr->as.number);
		return;

	case EXPR_VARIABLE:
		PrintIndent(level);
		if (neg) putchar('-');
		printf("%.*s\n", (int)expr->as.variable.ident.len, expr->as.variable.ident.chars);
		return;

	case EXPR_BINOP:
		PrintIndent(level);
		if (neg) putchar('-');
		printf("(%c\n", (char)expr->as.binop.op);
		PrintExprS_(expr->as.binop.lhs, level + 1);
		PrintExprS_(expr->as.binop.rhs, level + 1);
		PrintIndent(level);
		puts(")");
		return;

	case EXPR_SEQUENCE:
	{
		PrintIndent(level);
		puts("(seq");
		const ExprSeq *s = &expr->as.seq;
		while (s) {
			PrintExprS_(s->expr, level + 1);
			s = s->next;
		}
		PrintIndent(level);
		puts(")");
		return;
	}

	case EXPR_PARSE_ERROR:
		PrintIndent(level);
		if (neg) putchar('-');
		printf("<parse error: %s at %d:%d>\n", expr->as.error.message, expr->as.error.line, expr->as.error.column);
		return;

	default:
		assert(0 && "Invalid expr type");
	}
}

void PrintExprS(Expr *expr)
{
	PrintExprS_(expr, 0);
}