#include "lexer.h"

#include <assert.h>
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int RemainingChars(Lexer *lex)
{
	return (int)(lex->length - lex->pos.at);
}

static char PeekChar(Lexer *lex)
{
	if (RemainingChars(lex) <= 0)
		return 0;
	return lex->base[lex->pos.at];
}

static char Advance(Lexer *lex)
{
	if (RemainingChars(lex) <= 0)
		return 0;

	char c = lex->base[lex->pos.at++];

	if (c == '\n')
	{
		lex->pos.lineStart = lex->pos.at;
		++lex->pos.lineCount;
	}

	return c;
}

static void EatSpace(Lexer *lex)
{
	while (isspace(PeekChar(lex))) Advance(lex);
}

int GetColumn(Lexer *lex)
{
	int result = (int)(lex->pos.at - lex->pos.lineStart);
	assert(result >= 0);
	return result;
}

Lexer LexerFromCStr(const char *str)
{
	Lexer l = {0};
	l.base = str;
	l.length = (int)strlen(str);
	return l;
}

static void
NumberToken(Lexer *lex, Token *outToken)
{
	char buf[128] = {0};
	int tokStart = lex->pos.at;

	// TODO(jkk): only one radix point please.
	char c;
	while ((c = PeekChar(lex)) && isdigit(c) || c == '.') {
		Advance(lex);
	}

	unsigned long copyLength = (unsigned long)(lex->pos.at - tokStart) & (sizeof(buf) - 1);
	memcpy(buf, lex->base + tokStart, copyLength);
	buf[copyLength] = '\0';

	outToken->type = TOK_NUMBER;
	outToken->as.number = strtod(buf, NULL);
}

static void
IdentToken(Lexer *lex, Token *outToken)
{
	int tokStart = lex->pos.at;
	char c = PeekChar(lex);

	if (isalnum(c) || c == '_')
	{
		do
		{
			Advance(lex);
		} while (isalnum(c = PeekChar(lex)) || c == '_' || isdigit(c));
	}

	Ident ident = {0};
	ident.len = lex->pos.at - tokStart;
	ident.chars = calloc(ident.len + 1, sizeof(*ident.chars));
	memcpy(ident.chars, lex->base + tokStart, ident.len);

	outToken->type = TOK_IDENT;
	outToken->as.ident = ident;
}

Token
LexerNextToken(Lexer *lex)
{
	EatSpace(lex);

	Token token = {0};
	token.line = lex->pos.lineCount;
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
