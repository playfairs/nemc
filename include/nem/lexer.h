#ifndef NEM_LEXER_H
#define NEM_LEXER_H

#include <stddef.h>

#include "nem/source.h"

typedef enum {
    TOKEN_EOF,
    TOKEN_IDENT,
    TOKEN_STRING,
    TOKEN_INT,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_COMMA,
    TOKEN_SEMICOLON,
    TOKEN_COLON,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_EQUAL,
    TOKEN_ARROW,
    TOKEN_FN,
    TOKEN_LET,
    TOKEN_RETURN,
    TOKEN_INT_TYPE,
    TOKEN_STRING_TYPE,
    TOKEN_PRINT,
    TOKEN_ERROR
} TokenKind;

typedef struct {
    TokenKind kind;
    Span span;
    char *text;
} Token;

typedef struct {
    Token *items;
    size_t count;
    size_t capacity;
} TokenList;

TokenList *lex_source(const Source *source);
void tokens_free(TokenList *tokens);
const char *token_kind_name(TokenKind kind);

#endif
