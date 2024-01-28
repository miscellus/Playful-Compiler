#ifndef TOKENIZER_H
#define TOKENIZER_H

typedef enum TokenType_t
{
	TOK_END_OF_STREAM = 0,
	TOK_NUMBER = 256
} TokenType;

typedef struct Token_t
{
	int type;
	double number;
} Token;

typedef struct TokenStream_t
{
	const char *at;
	const char *const end;
} TokenStream;

TokenStream TokenStreamFromCStr(const char *str);

Token NextToken(TokenStream *ts);

#endif