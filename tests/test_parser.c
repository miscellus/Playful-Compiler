#include <assert.h>
#include <math.h>
#include <stdbool.h>

#include "unity.h"
#include "unity_internals.h"
#include "../src/tokenizer.h"
#include "../src/parser.h"

void setUp() {}
void tearDown(){}

static Expr *ArrangeExpr(const char *cstr)
{
	TokenStream ts = TokenStreamFromCStr(cstr);
	return ParseExpression(&ts, 0, (Token){TOK_INPUT_END});
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
	TEST_ASSERT_EQUAL_INT32(EXPR_NUMBER, expr->type);
}

void TEST_ParseExpression_SingleAdditionBinop_1Lhs2Rhs(void)
{
	// Arrange, Act
	Expr *expr = ArrangeExpr("1 + 2");

	// Assert
	TEST_ASSERT_NOT_NULL(expr);
	TEST_ASSERT_EQUAL_INT32(EXPR_BINOP, expr->type);
	Expr *lhs = expr->as.binop.lhs;
	Expr *rhs = expr->as.binop.rhs;
	TEST_ASSERT_EQUAL_INT32('+', expr->as.binop.op);

	TEST_ASSERT_NOT_NULL(lhs);
	TEST_ASSERT_NOT_NULL(rhs);
	TEST_ASSERT_EQUAL_INT32(EXPR_NUMBER, lhs->type);
	TEST_ASSERT_EQUAL_INT32(EXPR_NUMBER, rhs->type);
	TEST_ASSERT_EQUAL_DOUBLE(1.0, lhs->as.number);
	TEST_ASSERT_EQUAL_DOUBLE(2.0, rhs->as.number);
}

void TEST_ParseExpression_UnaryMinusOnNumber_NegationFlagSet(void)
{
	// Arrange, Act
	Expr *expr = ArrangeExpr("-1");
	ExprFlags expectedFlags = EXPR_FLAG_NEGATED;

	// Assert
	TEST_ASSERT_EQUAL_INT32(expectedFlags, expr->flags);
}

void TEST_ParseExpression_UnaryMinusOnParenBinop_NegationFlagSet(void)
{
	// Arrange, Act
	Expr *expr = ArrangeExpr("-(1 + 1)");
	ExprFlags expectedFlags = EXPR_FLAG_NEGATED;

	// Assert
	TEST_ASSERT_NOT_NULL(expr);
	TEST_ASSERT_EQUAL_INT32(expectedFlags, expr->flags);
}

void TEST_ParseExpression_UnaryMinusOnExponent_ExponentNegated(void)
{
	// Arrange, Act
	Expr *expr = ArrangeExpr("2^-1");
	ExprFlags expectedFlags = EXPR_FLAG_NEGATED;

	// Assert
	TEST_ASSERT_NOT_NULL(expr);
	TEST_ASSERT_EQUAL_INT32(EXPR_BINOP, expr->type);
	TEST_ASSERT_NOT_NULL(expr->as.binop.rhs);
	TEST_ASSERT_EQUAL_INT32(EXPR_NUMBER, expr->as.binop.rhs->type);
	TEST_ASSERT_EQUAL_INT32(expectedFlags, expr->as.binop.rhs->flags);
}

void TEST_EvalExpr_ComplicatedExpression_Expected(void)
{
	// Arrange
	char *expr_s =
		"(3.14159^2*(6022.0-0.00000000000000000016)/(2.71828+0.57721))";

	Expr *expr = ArrangeExpr(expr_s);

	double expected_value = 18035.150250378;

	// Act
	double actual_value = EvalExpr(expr);

	// Assert
	TEST_ASSERT_EQUAL_DOUBLE(expected_value, actual_value);
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
	RUN_TEST(TEST_EvalExpr_ComplicatedExpression_Expected);
	return UNITY_END();
}