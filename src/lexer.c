#include "lexer.h"

#include <assert.h>
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int RemainingChars(Lexer *lex)
{
	return (int)(lex->end - lex->at);
}

static char PeekChar(Lexer *lex)
{
	if (RemainingChars(lex) <= 0)
		return 0;
	return *lex->at;
}

static char Advance(Lexer *lex)
{
	if (RemainingChars(lex) <= 0)
		return 0;

	char c = *lex->at++;

	if (c == '\n')
	{
		lex->lineStart = lex->at;
		++lex->lineCount;
	}

	return c;
}

static void EatSpace(Lexer *lex)
{
	while (lex->at < lex->end && isspace(*lex->at)) Advance(lex);
}

int GetColumn(Lexer *lex)
{
	int result = (int)(lex->at - lex->lineStart);
	assert(result >= 0);
	return result;
}

Lexer LexerFromCStr(const char *str)
{
	return (Lexer){str, str + strlen(str), str, 0};
}

static void
NumberToken(Lexer *lex, Token *outToken)
{
	char buf[128] = {0};
	const char *tokStart = lex->at;

	// TODO(jkk): only one radix point please.
	char c;
	while ((c = PeekChar(lex)) && isdigit(c) || c == '.') {
		Advance(lex);
	}

	unsigned long copyLength = (unsigned long)(lex->at - tokStart) & (sizeof(buf) - 1);
	memcpy(buf, tokStart, copyLength);
	buf[copyLength] = '\0';

	outToken->type = TOK_NUMBER;
	outToken->as.number = strtod(buf, NULL);
}

static void
IdentToken(Lexer *lex, Token *outToken)
{
	const char *tokStart = lex->at;
	char c = PeekChar(lex);

	if (isalnum(c) || c == '_')
	{
		do
		{
			Advance(lex);
		} while (isalnum(c = PeekChar(lex)) || c == '_' || isdigit(c));
	}

	Ident ident = {0};
	ident.len = (unsigned long)(lex->at - tokStart);
	ident.chars = calloc(ident.len + 1, sizeof(*ident.chars));
	memcpy(ident.chars, tokStart, ident.len);

	outToken->type = TOK_IDENT;
	outToken->as.ident = ident;
}

Token
LexerNextToken(Lexer *lex)
{
	EatSpace(lex);

	Token token = {0};
	token.line = lex->lineCount;
	token.column = GetColumn(lex);

	char c = PeekChar(lex);

	if (isdigit(c)) {
		NumberToken(lex, &token);
	}
	else if (isalpha(c)) {
		IdentToken(lex, &token);
	}
	else if (c == '\0') {
		token.type = TOK_INPUT_END;
	}
	else {
		token.type = c;
		Advance(lex);
	}

	lex->token = token;
	return token;
}
