#include "tokenizer.h"

#include <assert.h>
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int
RemainingChars(TokenStream *ts)
{
	return ts->end - ts->at;
}

static char
PeekChar(TokenStream *ts)
{
	if (RemainingChars(ts) > 0) {
		return *ts->at;
	}

	return '\0';
}

static char
NextChar(TokenStream *ts)
{
	assert(ts->at < ts->end);
	if (*ts->at++ == '\n') {
		ts->lineStart = ts->at;
		++ts->lineCount;
	}
	return *ts->at;
}

static void
EatSpace(TokenStream *ts)
{
	while (ts->at < ts->end && isspace(*ts->at)) NextChar(ts);
}

int
GetColumn(TokenStream *ts)
{
	int result = (int)(ts->at - ts->lineStart);
	assert(result >= 0);
	return result;
}

TokenStream
TokenStreamFromCStr(const char *str)
{
	return (TokenStream){str, str + strlen(str), str, 0};
}

static void
NumberToken(TokenStream *ts, Token *outToken)
{
	char buf[128] = {0};
	const char *tokStart = ts->at;

	char c;
	do {
		c = NextChar(ts);
	} while (isdigit(c) || c == '.'); // TODO(jkk): only one radix point please.

	unsigned long copyLength = (unsigned long)(ts->at - tokStart) & (sizeof(buf) - 1);
	memcpy(buf, tokStart, copyLength);
	buf[copyLength] = '\0';

	outToken->h.type = TOK_NUMBER;
	outToken->u.number = strtod(buf, NULL);
}

static void
IdentToken(TokenStream *ts, Token *outToken)
{
	const char *tokStart = ts->at;

	char c;
	do {
		c = NextChar(ts);
	} while (isalnum(c) || c == '_');

	unsigned long copyLength = (unsigned long)(ts->at - tokStart);
	outToken->h.type = TOK_IDENT;
	outToken->u.ident = calloc(1, (copyLength + 1) * sizeof(*outToken->u.ident));
	memcpy((void *)outToken->u.ident, tokStart, copyLength);
}

Token
NextToken(TokenStream *ts)
{
	EatSpace(ts);

	Token token = {0};
	token.h.line = ts->lineCount;
	token.h.column = GetColumn(ts);

	char c = PeekChar(ts);

	if (isdigit(c)) {
		NumberToken(ts, &token);
	}
	else if (isalpha(c)) {
		IdentToken(ts, &token);
	}
	else if (c == '\0') {
		token.h.type = TOK_INPUT_END;
	}
	else {
		token.h.type = c;
		NextChar(ts);
	}

	return token;
}
