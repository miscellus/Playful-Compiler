#include "unity.h"
#include "unity_internals.h"
#include "../src/tokenizer.h"
#include "../src/parser.h"

void setUp() {}
void tearDown(){}

static Expr *ArrangeExpr(const char *cstr)
{
	TokenStream ts = TokenStreamFromCStr(cstr);
	return ParseExpression(&ts, 0, (Token){TOK_END_OF_STREAM});
}

void TEST_ParseExpression_EmptyInput_Null(void)
{
	// Arrange, Act
	Expr *expr = ArrangeExpr("");

	// Assert
	TEST_ASSERT_EQUAL_PTR(NULL, expr);
}

void TEST_ParseExpression_NumberWithSpaces_SingleNumberExpression(void)
{
	// Arrange, Act
	Expr *expr = ArrangeExpr(" 42 ");

	// Assert
	TEST_ASSERT_NOT_NULL(expr);
	TEST_ASSERT_EQUAL_INT32(EXPR_NUMBER, expr->h.type);
}

void TEST_ParseExpression_SingleAdditionBinop_1Lhs2Rhs(void)
{
	// Arrange, Act
	Expr *expr = ArrangeExpr("1 + 2");

	// Assert
	TEST_ASSERT_NOT_NULL(expr);
	TEST_ASSERT_EQUAL_INT32(EXPR_BINOP, expr->h.type);
	Expr *lhs = expr->binop.v.lhs;
	Expr *rhs = expr->binop.v.rhs;
	TEST_ASSERT_EQUAL_INT32('+', expr->binop.v.op);

	TEST_ASSERT_NOT_NULL(lhs);
	TEST_ASSERT_NOT_NULL(rhs);
	TEST_ASSERT_EQUAL_INT32(EXPR_NUMBER, lhs->h.type);
	TEST_ASSERT_EQUAL_INT32(EXPR_NUMBER, rhs->h.type);
	TEST_ASSERT_EQUAL_DOUBLE(1.0, lhs->number.v);
	TEST_ASSERT_EQUAL_DOUBLE(2.0, rhs->number.v);
}

void TEST_ParseExpression_UnaryMinusOnNumber_NegationFlagSet(void)
{
	// Arrange, Act
	Expr *expr = ArrangeExpr("-1");
	ExprFlags expectedFlags = EXPR_FLAG_NEGATED;

	// Assert
	TEST_ASSERT_EQUAL_INT32(expectedFlags, expr->h.flags);
}

void TEST_ParseExpression_UnaryMinusOnParenBinop_NegationFlagSet(void)
{
	// Arrange, Act
	Expr *expr = ArrangeExpr("-(1 + 1)");
	ExprFlags expectedFlags = EXPR_FLAG_NEGATED;

	// Assert
	TEST_ASSERT_NOT_NULL(expr);
	TEST_ASSERT_EQUAL_INT32(expectedFlags, expr->h.flags);
}

void TEST_ParseExpression_UnaryMinusOnExponent_ExponentNegated(void)
{
	// Arrange, Act
	Expr *expr = ArrangeExpr("2^-1");
	ExprFlags expectedFlags = EXPR_FLAG_NEGATED;

	// Assert
	TEST_ASSERT_NOT_NULL(expr);
	TEST_ASSERT_EQUAL_INT32(EXPR_BINOP, expr->h.type);
	TEST_ASSERT_NOT_NULL(expr->binop.v.rhs);
	TEST_ASSERT_EQUAL_INT32(EXPR_NUMBER, expr->binop.v.rhs->h.type);
	TEST_ASSERT_EQUAL_INT32(expectedFlags, expr->binop.v.rhs->h.flags);
}


int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(TEST_ParseExpression_EmptyInput_Null);
	RUN_TEST(TEST_ParseExpression_NumberWithSpaces_SingleNumberExpression);
	RUN_TEST(TEST_ParseExpression_SingleAdditionBinop_1Lhs2Rhs);
	RUN_TEST(TEST_ParseExpression_UnaryMinusOnNumber_NegationFlagSet);
	RUN_TEST(TEST_ParseExpression_UnaryMinusOnParenBinop_NegationFlagSet);
	RUN_TEST(TEST_ParseExpression_UnaryMinusOnExponent_ExponentNegated);
	return UNITY_END();
}