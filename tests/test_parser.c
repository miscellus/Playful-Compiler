#include "unity.h"
#include "unity_internals.h"
#include "../src/tokenizer.h"
#include "../src/parser.h"

void setUp() {}
void tearDown(){}

static Expr *ExprFor(const char *cstr)
{
	TokenStream ts = TokenStreamFromCStr(cstr);
	return ParseExpression(&ts, 0, (Token){TOK_END_OF_STREAM});
}

void TEST_ParseExpression_EmptyInput_Null(void)
{
	// Arrange, Act
	Expr *expr = ExprFor("");

	// Assert
	TEST_ASSERT_EQUAL_PTR(NULL, expr);
}

void TEST_ParseExpression_NumberWithSpaces_SingleNumberExpression(void)
{
	// Arrange, Act
	Expr *expr = ExprFor(" 42 ");

	// Assert
	TEST_ASSERT_NOT_NULL(expr);
	TEST_ASSERT_EQUAL_INT32(EXPR_NUMBER, expr->type);
}

void TEST_ParseExpression_SingleAdditionBinop_1Lhs2Rhs(void)
{
	// Arrange, Act
	Expr *expr = ExprFor("1 + 2");

	// Assert
	TEST_ASSERT_NOT_NULL(expr);
	TEST_ASSERT_EQUAL_INT32(EXPR_BINOP, expr->type);
	Expr *lhs = expr->binop.v.lhs;
	Expr *rhs = expr->binop.v.rhs;
	TEST_ASSERT_EQUAL_INT32('+', expr->binop.v.op);

	TEST_ASSERT_NOT_NULL(lhs);
	TEST_ASSERT_NOT_NULL(rhs);
	TEST_ASSERT_EQUAL_INT32(EXPR_NUMBER, lhs->type);
	TEST_ASSERT_EQUAL_INT32(EXPR_NUMBER, rhs->type);
	TEST_ASSERT_EQUAL_DOUBLE(1.0, lhs->number.v);
	TEST_ASSERT_EQUAL_DOUBLE(2.0, rhs->number.v);
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(TEST_ParseExpression_EmptyInput_Null);
	RUN_TEST(TEST_ParseExpression_NumberWithSpaces_SingleNumberExpression);
	RUN_TEST(TEST_ParseExpression_SingleAdditionBinop_1Lhs2Rhs);
	return UNITY_END();
}