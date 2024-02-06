#ifndef TOKENIZER_H
#define TOKENIZER_H

typedef enum TokenType {
	TOK_INPUT_END = 0,
	TOK_PLUS = '+',
	TOK_MINUS = '-',
	TOK_ASTERISK = '*',
	TOK_SLASH = '/',
	TOK_HAT = '^',
	TOK_NUMBER = 256,
	TOK_IDENT
} TokenType;

typedef struct TokenHeader {
	TokenType type;
	int line;
	int column;
} TokenHeader;

typedef struct Token {
	TokenHeader h;
	union {
		double number;
		const char *ident; // TODO(jkk): string type
	} u;
} Token;

typedef struct TokenStream {
	const char *at;
	const char *const end;
	const char *lineStart;
	int lineCount;
} TokenStream;

TokenStream
TokenStreamFromCStr(const char *str);

Token
NextToken(TokenStream *ts);

int
GetColumn(TokenStream *ts);

#endif