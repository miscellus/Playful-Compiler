#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stddef.h>

typedef enum TokenType_t
{
	TOK_INPUT_END = 0,
	TOK_EQUALS = '=',
	TOK_PLUS = '+',
	TOK_MINUS = '-',
	TOK_ASTERISK = '*',
	TOK_SLASH = '/',
	TOK_HAT = '^',
	TOK_NUMBER = 256,
	TOK_IDENT,
} TokenType;

typedef struct Ident_t
{
	char *chars;
	size_t len;
} Ident;

typedef struct Token_t
{
	int type;
	int line;
	int column;
	union {
		double number;
		Ident ident;
	} as;
} Token;

typedef struct TokenStream_t
{
	const char *at;
	const char *const end;
	const char *lineStart;
	int lineCount;
} TokenStream;

TokenStream TokenStreamFromCStr(const char *str);

Token NextToken(TokenStream *ts);

int GetColumn(TokenStream *ts);

#endif