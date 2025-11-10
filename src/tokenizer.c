#include "tokenizer.h"

#include <assert.h>
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int RemainingChars(TokenStream *ts)
{
	return (int)(ts->end - ts->at);
}

static char PeekChar(TokenStream *ts)
{
	if (RemainingChars(ts) > 0)
	{
		return *ts->at;
	}
	return 0;
}

static char Advance(TokenStream *ts)
{
	assert(ts->at < ts->end);

	char c = *ts->at++;

	if (c == '\n')
	{
		ts->lineStart = ts->at;
		++ts->lineCount;
	}

	return c;
}

static void EatSpace(TokenStream *ts)
{
	while (ts->at < ts->end && isspace(*ts->at)) Advance(ts);
}

int GetColumn(TokenStream *ts)
{
	int result = (int)(ts->at - ts->lineStart);
	assert(result >= 0);
	return result;
}

TokenStream TokenStreamFromCStr(const char *str)
{
	return (TokenStream){str, str + strlen(str), str, 0};
}

static void
NumberToken(TokenStream *ts, Token *outToken)
{
	char buf[128] = {0};
	const char *tokStart = ts->at;

	// TODO(jkk): only one radix point please.
	char c;
	while ((c = PeekChar(ts)) && isdigit(c) || c == '.') {
		Advance(ts);
	}

	unsigned long copyLength = (unsigned long)(ts->at - tokStart) & (sizeof(buf) - 1);
	memcpy(buf, tokStart, copyLength);
	buf[copyLength] = '\0';

	outToken->type = TOK_NUMBER;
	outToken->as.number = strtod(buf, NULL);
}

static void
IdentToken(TokenStream *ts, Token *outToken)
{
	const char *tokStart = ts->at;

	char c;
	do {
		c = Advance(ts);
	} while (isalnum(c) || c == '_');

	Ident ident = {0};
	ident.len = (unsigned long)(ts->at - tokStart);
	ident.chars = calloc(ident.len + 1, sizeof(*ident.chars));
	memcpy(ident.chars, tokStart, ident.len);

	outToken->type = TOK_IDENT;
	outToken->as.ident = ident;
}

Token
NextToken(TokenStream *ts)
{
	EatSpace(ts);

	Token token = {0};
	token.line = ts->lineCount;
	token.column = GetColumn(ts);

	char c = PeekChar(ts);

	if (isdigit(c)) {
		NumberToken(ts, &token);
	}
	else if (isalpha(c)) {
		IdentToken(ts, &token);
	}
	else if (c == '\0') {
		token.type = TOK_INPUT_END;
	}
	else {
		token.type = c;
		Advance(ts);
	}

	return token;
}
