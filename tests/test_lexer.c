#include "unity.h"
#include "unity_internals.h"
#include "../src/lexer.h"

void setUp(){}
void tearDown(){}

void TEST_TokenStreamFromCStr_EmptyInput_EndAtStart(void)
{
	// Arrange, Act
	Lexer lex = LexerFromCStr("");

	// Assert
	TEST_ASSERT_EQUAL_INT32(lex.pos.at, lex.length);
}

void TEST_TokenStreamFromCStr_InputOfLength13_EndAtStartPlus13(void)
{
	// Arrange, Act
	Lexer lex = LexerFromCStr("Hello, World!");

	// Assert
	TEST_ASSERT_EQUAL_INT32(lex.length, 13);
}

void TEST_LexerNextToken_EmptyInput_EmptyOutput(void)
{
	// Arrange
	Lexer lex = LexerFromCStr("");

	// Act
	Token token = LexerNextToken(&lex);

	// Assert
	TEST_ASSERT_EQUAL_INT32(TOK_INPUT_END, token.type);
}

void TEST_LexerNextToken_NumberInInput_MatchingNumberToken(void)
{
	// Arrange
	Lexer lex = LexerFromCStr("42");

	// Act
	Token token = LexerNextToken(&lex);

	// Assert
	TEST_ASSERT_EQUAL_INT32(TOK_NUMBER, token.type);
	TEST_ASSERT_EQUAL_DOUBLE(42, token.as.number);
}


void TEST_LexerNextToken_CharactersBetween1And255_TokenTypeEqualsCharacterOrdinalValue(void)
{
	// Arrange
	Lexer lex = LexerFromCStr("+!@\xff");

	// Act
	Token tokPlus = LexerNextToken(&lex);
	Token tokExclaim = LexerNextToken(&lex);
	Token tokAt = LexerNextToken(&lex);
	Token tokHexFF = LexerNextToken(&lex);

	// Assert
	TEST_ASSERT_EQUAL_INT32('+', tokPlus.type);
	TEST_ASSERT_EQUAL_INT32('!', tokExclaim.type);
	TEST_ASSERT_EQUAL_INT32('@', tokAt.type);
	TEST_ASSERT_EQUAL_INT32('\xff', tokHexFF.type);
}

void TEST_LexerNextToken_SeveralLinesAndColumns_ExpectedLineAndColumn(void)
{
	// Arrange
	Lexer lex = LexerFromCStr("\n\n   .");
	int expectedLine = 2;
	int expectedColumn = 3;

	// Act
	Token token = LexerNextToken(&lex);

	// Assert
	TEST_ASSERT_EQUAL_INT32(expectedLine, token.line);
	TEST_ASSERT_EQUAL_INT32(expectedColumn, token.column);
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(TEST_TokenStreamFromCStr_EmptyInput_EndAtStart);
	RUN_TEST(TEST_TokenStreamFromCStr_InputOfLength13_EndAtStartPlus13);
	RUN_TEST(TEST_LexerNextToken_EmptyInput_EmptyOutput);
	RUN_TEST(TEST_LexerNextToken_NumberInInput_MatchingNumberToken);
	RUN_TEST(TEST_LexerNextToken_CharactersBetween1And255_TokenTypeEqualsCharacterOrdinalValue);
	RUN_TEST(TEST_LexerNextToken_SeveralLinesAndColumns_ExpectedLineAndColumn);
	return UNITY_END();
}