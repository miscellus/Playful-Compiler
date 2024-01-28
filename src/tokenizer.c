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

static char NextChar(TokenStream *ts)
{
	char c = PeekChar(ts);
	if (c) ++ts->at;
	return c;
}

static void EatSpace(TokenStream *ts)
{
	while (ts->at < ts->end && isspace(*ts->at)) ++ts->at;
}

TokenStream TokenStreamFromCStr(const char *str)
{
	return (TokenStream){str, str + strlen(str)};
}

Token NextToken(TokenStream *ts)
{
	EatSpace(ts);
	if (RemainingChars(ts) <= 0)
	{
		return (Token){TOK_END_OF_STREAM};
	}

	const char *tokStart = ts->at;

	Token result = {0};

	if (isdigit(PeekChar(ts)))
	{
		char buf[64] = {0};
		char c;
		while((c = PeekChar(ts), isdigit(c) || c == '.'))
		{
			++ts->at;
		}

		unsigned long copyLength = (unsigned long)(ts->at - tokStart) & 63;
		memcpy(buf, tokStart, copyLength);
		buf[copyLength] = '\0';
		result.type = TOK_NUMBER;
		result.number = strtod(buf, NULL);
	}
	else
	{
		result.type = NextChar(ts);
	}

	return result;
}
