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
	char oldChar = *ts->at;
	char c = *++ts->at;

	if (oldChar == '\n') {
		ts->lineStart = ts->at;
		++ts->lineCount;
	}

	return c;
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

Token
NextToken(TokenStream *ts)
{
	EatSpace(ts);

	Token token = {0};
	token.h.line = ts->lineCount;
	token.h.column = GetColumn(ts);

	if (RemainingChars(ts) <= 0) {
		token.h.type = TOK_INPUT_END;
		return token;
	}

	const char *tokStart = ts->at;

	char buf[128] = {0};
	char c = PeekChar(ts); // 1 + 1)

	if (isdigit(c)) {
		do {
			c = NextChar(ts);
		} while (isdigit(c) || c == '.'); // TODO(jkk): only one radix point please.

		unsigned long copyLength = (unsigned long)(ts->at - tokStart) & (sizeof(buf) - 1);
		memcpy(buf, tokStart, copyLength);
		buf[copyLength] = '\0';
		token.h.type = TOK_NUMBER;
		token.u.number = strtod(buf, NULL);
	}
	else if (isalpha(c)) {
		do {
			c = NextChar(ts);
		} while (isalnum(c) || c == '_');

		unsigned long copyLength = (unsigned long)(ts->at - tokStart) & (sizeof(buf) - 1);
		token.h.type = TOK_IDENT;
		token.u.ident = calloc(1, (copyLength + 1) * sizeof(*token.u.ident));
		memcpy((void *)token.u.ident, tokStart, copyLength);
	}
	else {
		token.h.type = c;
		NextChar(ts);
	}

	return token;
}
