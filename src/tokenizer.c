#include "tokenizer.h"

#include <assert.h>
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int RemainingChars(TokenStream *ts)
{
	return ts->end - ts->at;
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

static int GetColumn(TokenStream *ts)
{
	int result = (int)(ts->at - ts->lineStart);
	assert(result >= 0);
	return result;
}

TokenStream TokenStreamFromCStr(const char *str)
{
	return (TokenStream){str, str + strlen(str), str, 0};
}

Token NextToken(TokenStream *ts)
{
	EatSpace(ts);

	Token result = {0};
	result.line = ts->lineCount;
	result.column = GetColumn(ts);

	if (RemainingChars(ts) <= 0)
	{
		result.type = TOK_END_OF_STREAM;
		return result;
	}

	const char *tokStart = ts->at;

	if (isdigit(PeekChar(ts)))
	{
		char buf[64] = {0};
		char c;
		while((c = PeekChar(ts), isdigit(c) || c == '.'))
		{
			Advance(ts);
		}

		unsigned long copyLength = (unsigned long)(ts->at - tokStart) & 63;
		memcpy(buf, tokStart, copyLength);
		buf[copyLength] = '\0';
		result.type = TOK_NUMBER;
		result.number = strtod(buf, NULL);
	}
	else
	{
		result.type = Advance(ts);
	}

	return result;
}
