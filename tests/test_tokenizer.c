#include "unity.h"
#include "unity_internals.h"
#include "../src/tokenizer.h"

void setUp(){}
void tearDown(){}

void TEST_TokenStreamFromCStr_EmptyInput_EndAtStart(void)
{
	// Arrange, Act
	TokenStream ts = TokenStreamFromCStr("");

	// Assert
	TEST_ASSERT_EQUAL_PTR(ts.at, ts.end);
}

void TEST_TokenStreamFromCStr_InputOfLength13_EndAtStartPlus13(void)
{
	// Arrange, Act
	TokenStream ts = TokenStreamFromCStr("Hello, World!");

	// Assert
	TEST_ASSERT_EQUAL_PTR(ts.at + 13, ts.end);
}

void TEST_NextToken_EmptyInput_EmptyOutput(void)
{
	// Arrange
	TokenStream ts = TokenStreamFromCStr("");

	// Act
	Token token = NextToken(&ts);

	// Assert
	TEST_ASSERT_EQUAL_INT32(TOK_END_OF_STREAM, token.type);
}

void TEST_NextToken_NumberInInput_MatchingNumberToken(void)
{
	// Arrange
	TokenStream ts = TokenStreamFromCStr("42");

	// Act
	Token token = NextToken(&ts);

	// Assert
	TEST_ASSERT_EQUAL_INT32(TOK_NUMBER, token.type);
	TEST_ASSERT_EQUAL_DOUBLE(42, token.number);
}


void TEST_NextToken_CharactersBetween1And255_TokenTypeEqualsCharacterOrdinalValue(void)
{
	// Arrange
	TokenStream ts = TokenStreamFromCStr("+!@\xff");

	// Act
	Token tokPlus = NextToken(&ts);
	Token tokExclaim = NextToken(&ts);
	Token tokAt = NextToken(&ts);
	Token tokHexFF = NextToken(&ts);

	// Assert
	TEST_ASSERT_EQUAL_INT32('+', tokPlus.type);
	TEST_ASSERT_EQUAL_INT32('!', tokExclaim.type);
	TEST_ASSERT_EQUAL_INT32('@', tokAt.type);
	TEST_ASSERT_EQUAL_INT32('\xff', tokHexFF.type);
}

void TEST_NextToken_SeveralLinesAndColumns_ExpectedLineAndColumn(void)
{
	// Arrange
	TokenStream ts = TokenStreamFromCStr("\n\n   .");
	int expectedLine = 2;
	int expectedColumn = 3;

	// Act
	Token token = NextToken(&ts);

	// Assert
	TEST_ASSERT_EQUAL_INT32(expectedLine, token.line);
	TEST_ASSERT_EQUAL_INT32(expectedColumn, token.column);
}

int main(void)
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