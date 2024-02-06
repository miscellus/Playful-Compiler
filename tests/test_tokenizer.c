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

void
TEST_TokenStreamFromCStr_EmptyInput_EndAtStart(void)
{
	// Arrange, Act
	TokenStream ts = TokenStreamFromCStr("");

	// Assert
	TEST_ASSERT_EQUAL_PTR(ts.at, ts.end);
}

void
TEST_TokenStreamFromCStr_InputOfLength13_EndAtStartPlus13(void)
{
	// Arrange, Act
	TokenStream ts = TokenStreamFromCStr("Hello, World!");

	// Assert
	TEST_ASSERT_EQUAL_PTR(ts.at + 13, ts.end);
}

void
TEST_NextToken_EmptyInput_EmptyOutput(void)
{
	// Arrange
	TokenStream ts = TokenStreamFromCStr("");

	// Act
	Token token = NextToken(&ts);

	// Assert
	TEST_ASSERT_EQUAL_INT32(TOK_INPUT_END, token.h.type);
}

void
TEST_NextToken_NumberInInput_MatchingNumberToken(void)
{
	// Arrange
	TokenStream ts = TokenStreamFromCStr("42");

	// Act
	Token token = NextToken(&ts);

	// Assert
	TEST_ASSERT_EQUAL_INT32(TOK_NUMBER, token.h.type);
	TEST_ASSERT_EQUAL_DOUBLE(42, token.u.number);
}

void
TEST_NextToken_CharactersBetween1And255_TokenTypeEqualsCharacterOrdinalValue(void)
{
	// Arrange
	TokenStream ts = TokenStreamFromCStr("+!@\xff");

	// Act
	Token tokPlus = NextToken(&ts);
	Token tokExclaim = NextToken(&ts);
	Token tokAt = NextToken(&ts);
	Token tokHexFF = NextToken(&ts);

	// Assert
	TEST_ASSERT_EQUAL_INT32('+', tokPlus.h.type);
	TEST_ASSERT_EQUAL_INT32('!', tokExclaim.h.type);
	TEST_ASSERT_EQUAL_INT32('@', tokAt.h.type);
	TEST_ASSERT_EQUAL_INT32('\xff', tokHexFF.h.type);
}

void
TEST_NextToken_SeveralLinesAndColumns_ExpectedLineAndColumn(void)
{
	// Arrange
	TokenStream ts = TokenStreamFromCStr("\n\n   .");
	int expectedLine = 2;
	int expectedColumn = 3;

	// Act
	Token token = NextToken(&ts);

	// Assert
	TEST_ASSERT_EQUAL_INT32(expectedLine, token.h.line);
	TEST_ASSERT_EQUAL_INT32(expectedColumn, token.h.column);
}

int
main(void)
{
	UNITY_BEGIN();
	RUN_TEST(TEST_TokenStreamFromCStr_EmptyInput_EndAtStart);
	RUN_TEST(TEST_TokenStreamFromCStr_InputOfLength13_EndAtStartPlus13);
	RUN_TEST(TEST_NextToken_EmptyInput_EmptyOutput);
	RUN_TEST(TEST_NextToken_NumberInInput_MatchingNumberToken);
	RUN_TEST(TEST_NextToken_CharactersBetween1And255_TokenTypeEqualsCharacterOrdinalValue);
	RUN_TEST(TEST_NextToken_SeveralLinesAndColumns_ExpectedLineAndColumn);
	return UNITY_END();
}