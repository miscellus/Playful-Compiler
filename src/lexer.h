#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stddef.h>

typedef enum TokenType_t
{
	TOK_NO_TOKEN = 0,
	TOK_INPUT_END,
	TOK_EQUALS = '=',
	TOK_PLUS = '+',
	TOK_MINUS = '-',
	TOK_ASTERISK = '*',
	TOK_SLASH = '/',
	TOK_HAT = '^',
	TOK_SEMICOLON = ';',
	TOK_OPAREN = '(',
	TOK_CPAREN = ')',
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
	TokenType type;
	int line;
	int column;
	union {
		double number;
		Ident ident;
	} as;
} Token;

typedef struct Lexer_t
{
	const char *at;
	const char *end;
	const char *lineStart;
	int lineCount;
	Token token;
} Lexer;

Lexer LexerFromCStr(const char *str);

Token LexerNextToken(Lexer *lex);

int GetColumn(Lexer *lex);

#endif