#include "nem/lexer.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static void token_list_init(TokenList *tokens) {
    tokens->items = NULL;
    tokens->count = 0;
    tokens->capacity = 0;
}

static void token_list_push(TokenList *tokens, Token token) {
    if (tokens->count == tokens->capacity) {
        size_t new_capacity = tokens->capacity == 0 ? 8 : tokens->capacity * 2;
        Token *new_items = realloc(tokens->items, new_capacity * sizeof(Token));
        if (new_items == NULL) {
            return;
        }
        tokens->items = new_items;
        tokens->capacity = new_capacity;
    }
    tokens->items[tokens->count++] = token;
}

static char *copy_slice(const char *text, size_t start, size_t length) {
    char *buffer = malloc(length + 1);
    if (buffer == NULL) {
        return NULL;
    }
    memcpy(buffer, text + start, length);
    buffer[length] = '\0';
    return buffer;
}

static int is_keyword(const char *text, const char *keyword) {
    return strcmp(text, keyword) == 0;
}

TokenList *lex_source(const Source *source) {
    TokenList *tokens = malloc(sizeof(TokenList));
    if (tokens == NULL) {
        return NULL;
    }
    token_list_init(tokens);

    size_t index = 0;
    size_t line = 1;
    size_t column = 1;
    while (index < source->length) {
        char current = source->text[index];
        if (isspace((unsigned char)current)) {
            if (current == '\n') {
                line += 1;
                column = 1;
            } else {
                column += 1;
            }
            index += 1;
            continue;
        }

        Span span;
        span_init(&span, index, 1, line, column);
        Token token = { .kind = TOKEN_ERROR, .span = span, .text = NULL };

        if (current == '{') {
            token.kind = TOKEN_LBRACE;
            token.text = strdup("{");
            index += 1;
            column += 1;
        } else if (current == '}') {
            token.kind = TOKEN_RBRACE;
            token.text = strdup("}");
            index += 1;
            column += 1;
        } else if (current == '(') {
            token.kind = TOKEN_LPAREN;
            token.text = strdup("(");
            index += 1;
            column += 1;
        } else if (current == ')') {
            token.kind = TOKEN_RPAREN;
            token.text = strdup(")");
            index += 1;
            column += 1;
        } else if (current == ',') {
            token.kind = TOKEN_COMMA;
            token.text = strdup(",");
            index += 1;
            column += 1;
        } else if (current == ';') {
            token.kind = TOKEN_SEMICOLON;
            token.text = strdup(";");
            index += 1;
            column += 1;
        } else if (current == ':') {
            token.kind = TOKEN_COLON;
            token.text = strdup(":");
            index += 1;
            column += 1;
        } else if (current == '+') {
            token.kind = TOKEN_PLUS;
            token.text = strdup("+");
            index += 1;
            column += 1;
        } else if (current == '-') {
            if (index + 1 < source->length && source->text[index + 1] == '>') {
                token.kind = TOKEN_ARROW;
                token.text = strdup("->");
                index += 2;
                column += 2;
            } else {
                token.kind = TOKEN_MINUS;
                token.text = strdup("-");
                index += 1;
                column += 1;
            }
        } else if (current == '*') {
            token.kind = TOKEN_STAR;
            token.text = strdup("*");
            index += 1;
            column += 1;
        } else if (current == '/') {
            token.kind = TOKEN_SLASH;
            token.text = strdup("/");
            index += 1;
            column += 1;
        } else if (current == '=') {
            if (index + 1 < source->length && source->text[index + 1] == '=') {
                token.kind = TOKEN_EQUAL_EQUAL;
                token.text = strdup("==");
                index += 2;
                column += 2;
            } else {
                token.kind = TOKEN_EQUAL;
                token.text = strdup("=");
                index += 1;
                column += 1;
            }
        } else if (current == '!') {
            if (index + 1 < source->length && source->text[index + 1] == '=') {
                token.kind = TOKEN_NOT_EQUAL;
                token.text = strdup("!=");
                index += 2;
                column += 2;
            } else {
                token.kind = TOKEN_BANG;
                token.text = strdup("!");
                index += 1;
                column += 1;
            }
        } else if (current == '<') {
            if (index + 1 < source->length && source->text[index + 1] == '=') {
                token.kind = TOKEN_LESS_EQUAL;
                token.text = strdup("<=");
                index += 2;
                column += 2;
            } else {
                token.kind = TOKEN_LESS;
                token.text = strdup("<");
                index += 1;
                column += 1;
            }
        } else if (current == '>') {
            if (index + 1 < source->length && source->text[index + 1] == '=') {
                token.kind = TOKEN_GREATER_EQUAL;
                token.text = strdup(">=");
                index += 2;
                column += 2;
            } else {
                token.kind = TOKEN_GREATER;
                token.text = strdup(">");
                index += 1;
                column += 1;
            }
        } else if (current == '&') {
            if (index + 1 < source->length && source->text[index + 1] == '&') {
                token.kind = TOKEN_AMP_AMP;
                token.text = strdup("&&");
                index += 2;
                column += 2;
            } else {
                token.kind = TOKEN_ERROR;
                token.text = strdup("&");
                token_list_push(tokens, token);
                return tokens;
            }
        } else if (current == '|') {
            if (index + 1 < source->length && source->text[index + 1] == '|') {
                token.kind = TOKEN_PIPE_PIPE;
                token.text = strdup("||");
                index += 2;
                column += 2;
            } else {
                token.kind = TOKEN_ERROR;
                token.text = strdup("|");
                token_list_push(tokens, token);
                return tokens;
            }
        } else if (current == '"') {
            size_t start = index;
            index += 1;
            column += 1;
            while (index < source->length && source->text[index] != '"') {
                if (source->text[index] == '\\') {
                    index += 1;
                    column += 1;
                }
                index += 1;
                column += 1;
            }
            if (index >= source->length) {
                token.kind = TOKEN_ERROR;
                token.text = strdup("unterminated string");
                token_list_push(tokens, token);
                return tokens;
            }
            index += 1;
            column += 1;
            span.length = index - start;
            token.kind = TOKEN_STRING;
            token.text = copy_slice(source->text, start + 1, (index - start) - 2);
        } else if (isdigit((unsigned char)current)) {
            size_t start = index;
            while (index < source->length && isdigit((unsigned char)source->text[index])) {
                index += 1;
                column += 1;
            }
            span.length = index - start;
            token.kind = TOKEN_INT;
            token.text = copy_slice(source->text, start, span.length);
        } else if (isalpha((unsigned char)current) || current == '_') {
            size_t start = index;
            while (index < source->length && (isalnum((unsigned char)source->text[index]) || source->text[index] == '_')) {
                index += 1;
                column += 1;
            }
            span.length = index - start;
            token.text = copy_slice(source->text, start, span.length);
            if (is_keyword(token.text, "fn")) {
                token.kind = TOKEN_FN;
            } else if (is_keyword(token.text, "let")) {
                token.kind = TOKEN_LET;
            } else if (is_keyword(token.text, "mut")) {
                token.kind = TOKEN_MUT;
            } else if (is_keyword(token.text, "return")) {
                token.kind = TOKEN_RETURN;
            } else if (is_keyword(token.text, "if")) {
                token.kind = TOKEN_IF;
            } else if (is_keyword(token.text, "else")) {
                token.kind = TOKEN_ELSE;
            } else if (is_keyword(token.text, "while")) {
                token.kind = TOKEN_WHILE;
            } else if (is_keyword(token.text, "true")) {
                token.kind = TOKEN_TRUE;
            } else if (is_keyword(token.text, "false")) {
                token.kind = TOKEN_FALSE;
            } else if (is_keyword(token.text, "int")) {
                token.kind = TOKEN_INT_TYPE;
            } else if (is_keyword(token.text, "string")) {
                token.kind = TOKEN_STRING_TYPE;
            } else if (is_keyword(token.text, "bool")) {
                token.kind = TOKEN_BOOL_TYPE;
            } else if (is_keyword(token.text, "print")) {
                token.kind = TOKEN_PRINT;
            } else {
                token.kind = TOKEN_IDENT;
            }
        } else {
            token.kind = TOKEN_ERROR;
            token.text = copy_slice(source->text, index, 1);
            token_list_push(tokens, token);
            return tokens;
        }

        token.span = span;
        token_list_push(tokens, token);
    }

    Token eof = { .kind = TOKEN_EOF, .span = {0}, .text = strdup("<eof>") };
    token_list_push(tokens, eof);
    return tokens;
}

void tokens_free(TokenList *tokens) {
    if (tokens == NULL) {
        return;
    }
    for (size_t i = 0; i < tokens->count; ++i) {
        free(tokens->items[i].text);
    }
    free(tokens->items);
    free(tokens);
}

const char *token_kind_name(TokenKind kind) {
    switch (kind) {
        case TOKEN_EOF: return "EOF";
        case TOKEN_IDENT: return "IDENT";
        case TOKEN_STRING: return "STRING";
        case TOKEN_INT: return "INT";
        case TOKEN_LBRACE: return "{";
        case TOKEN_RBRACE: return "}";
        case TOKEN_LPAREN: return "(";
        case TOKEN_RPAREN: return ")";
        case TOKEN_COMMA: return ",";
        case TOKEN_SEMICOLON: return ";";
        case TOKEN_COLON: return ":";
        case TOKEN_PLUS: return "+";
        case TOKEN_MINUS: return "-";
        case TOKEN_STAR: return "*";
        case TOKEN_SLASH: return "/";
        case TOKEN_EQUAL: return "=";
        case TOKEN_EQUAL_EQUAL: return "==";
        case TOKEN_NOT_EQUAL: return "!=";
        case TOKEN_LESS: return "<";
        case TOKEN_LESS_EQUAL: return "<=";
        case TOKEN_GREATER: return ">";
        case TOKEN_GREATER_EQUAL: return ">=";
        case TOKEN_BANG: return "!";
        case TOKEN_AMP_AMP: return "&&";
        case TOKEN_PIPE_PIPE: return "||";
        case TOKEN_ARROW: return "->";
        case TOKEN_FN: return "fn";
        case TOKEN_LET: return "let";
        case TOKEN_MUT: return "mut";
        case TOKEN_RETURN: return "return";
        case TOKEN_IF: return "if";
        case TOKEN_ELSE: return "else";
        case TOKEN_WHILE: return "while";
        case TOKEN_TRUE: return "true";
        case TOKEN_FALSE: return "false";
        case TOKEN_INT_TYPE: return "int";
        case TOKEN_STRING_TYPE: return "string";
        case TOKEN_BOOL_TYPE: return "bool";
        case TOKEN_PRINT: return "print";
        case TOKEN_ERROR: return "ERROR";
    }
    return "UNKNOWN";
}
