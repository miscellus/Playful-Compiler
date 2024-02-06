#include "../src/parser.h"
#include "../src/tokenizer.h"
#include "unity.h"
#include "unity_internals.h"

void
setUp()
{
}
void
tearDown()
{
}

static Expr *
ArrangeExpr(const char *cstr)
{
	TokenStream ts = TokenStreamFromCStr(cstr);
	return ParseExpression(&ts);
}

void
TEST_ParseExpression_EmptyInput_UnitExpression(void)
{
	// Arrange, Act
	Expr *expr = ArrangeExpr("");

	// Assert
	TEST_ASSERT_EQUAL_PTR(EXPR_UNIT, expr->h.type);
}

void
TEST_ParseExpression_BinaryOperatorWithMissingRightHandOperand_ErrorWithExpectedMessage(void)
{
	// Arrange, Act
	Expr *expr = ArrangeExpr("1 + ");

	// Assert
	TEST_ASSERT_EQUAL_INT32(EXPR_ERROR, expr->h.type);
	TEST_ASSERT_EQUAL_STRING("Operator '+' missing right hand operand", expr->u.error.message);
}

void
TEST_ParseExpression_BinaryOperatorWithMissingLeftHandOperand_ErrorWithExpectedMessage(void)
{
	// Arrange, Act
	Expr *expr = ArrangeExpr("* 1");

	// Assert
	TEST_ASSERT_EQUAL_INT32(EXPR_ERROR, expr->h.type);
	TEST_ASSERT_EQUAL_STRING("Unexpected token, '*'", expr->u.error.message);
}

void
TEST_ParseExpression_NumberWithSpaces_SingleNumberExpression(void)
{
	// Arrange, Act
	Expr *expr = ArrangeExpr(" 42 ");

	// Assert
	TEST_ASSERT_NOT_NULL(expr);
	TEST_ASSERT_EQUAL_INT32(EXPR_NUMBER, expr->h.type);
}

void
TEST_ParseExpression_SingleAdditionBinop_1Lhs2Rhs(void)
{
	// Arrange, Act
	Expr *expr = ArrangeExpr("1 + 2");

	// Assert
	TEST_ASSERT_NOT_NULL(expr);
	TEST_ASSERT_EQUAL_INT32(EXPR_BINOP, expr->h.type);
	Expr *lhs = expr->u.binop.lhs;
	Expr *rhs = expr->u.binop.rhs;
	TEST_ASSERT_EQUAL_INT32('+', expr->u.binop.op);

	TEST_ASSERT_NOT_NULL(lhs);
	TEST_ASSERT_NOT_NULL(rhs);
	TEST_ASSERT_EQUAL_INT32(EXPR_NUMBER, lhs->h.type);
	TEST_ASSERT_EQUAL_INT32(EXPR_NUMBER, rhs->h.type);
	TEST_ASSERT_EQUAL_DOUBLE(1.0, lhs->u.number);
	TEST_ASSERT_EQUAL_DOUBLE(2.0, rhs->u.number);
}

void
TEST_ParseExpression_UnaryMinusOnNumber_NegationFlagSet(void)
{
	// Arrange, Act
	Expr *expr = ArrangeExpr("-1");
	ExprFlags expectedFlags = EXPR_FLAG_NEGATED;

	// Assert
	TEST_ASSERT_EQUAL_INT32(expectedFlags, expr->h.flags);
}

void
TEST_ParseExpression_UnaryMinusOnParenBinop_NegationFlagSet(void)
{
	// Arrange, Act
	Expr *expr = ArrangeExpr("-(1 + 1)");
	ExprFlags expectedFlags = EXPR_FLAG_NEGATED;

	// Assert
	TEST_ASSERT_NOT_NULL(expr);
	TEST_ASSERT_EQUAL_INT32(expectedFlags, expr->h.flags);
}

void
TEST_ParseExpression_UnaryMinusOnExponent_ExponentNegated(void)
{
	// Arrange, Act
	Expr *expr = ArrangeExpr("2^-1");
	ExprFlags expectedFlags = EXPR_FLAG_NEGATED;

	// Assert
	TEST_ASSERT_NOT_NULL(expr);
	TEST_ASSERT_EQUAL_INT32(EXPR_BINOP, expr->h.type);
	TEST_ASSERT_NOT_NULL(expr->u.binop.rhs);
	TEST_ASSERT_EQUAL_INT32(EXPR_NUMBER, expr->u.binop.rhs->h.type);
	TEST_ASSERT_EQUAL_INT32(expectedFlags, expr->u.binop.rhs->h.flags);
}

int
main(void)
{
	UNITY_BEGIN();
	RUN_TEST(TEST_ParseExpression_EmptyInput_UnitExpression);
	RUN_TEST(TEST_ParseExpression_BinaryOperatorWithMissingRightHandOperand_ErrorWithExpectedMessage);
	RUN_TEST(TEST_ParseExpression_BinaryOperatorWithMissingLeftHandOperand_ErrorWithExpectedMessage);
	RUN_TEST(TEST_ParseExpression_NumberWithSpaces_SingleNumberExpression);
	RUN_TEST(TEST_ParseExpression_SingleAdditionBinop_1Lhs2Rhs);
	RUN_TEST(TEST_ParseExpression_UnaryMinusOnNumber_NegationFlagSet);
	RUN_TEST(TEST_ParseExpression_UnaryMinusOnParenBinop_NegationFlagSet);
	RUN_TEST(TEST_ParseExpression_UnaryMinusOnExponent_ExponentNegated);
	return UNITY_END();
}