#include "nem/parser.h"

#include <stdlib.h>
#include <string.h>

typedef struct {
    const TokenList *tokens;
    size_t index;
} Parser;

static const Token *parser_peek(const Parser *parser) {
    if (parser->index >= parser->tokens->count) {
        return &parser->tokens->items[parser->tokens->count - 1];
    }
    return &parser->tokens->items[parser->index];
}

static const Token *parser_next(Parser *parser) {
    if (parser->index >= parser->tokens->count) {
        return &parser->tokens->items[parser->tokens->count - 1];
    }
    return &parser->tokens->items[parser->index++];
}

static int parser_accept(Parser *parser, TokenKind kind) {
    if (parser_peek(parser)->kind == kind) {
        parser_next(parser);
        return 1;
    }
    return 0;
}

static const Token *parser_expect(Parser *parser, TokenKind kind) {
    const Token *token = parser_peek(parser);
    if (token->kind != kind) {
        return NULL;
    }
    parser_next(parser);
    return token;
}

static Type parse_type(Parser *parser) {
    if (parser_accept(parser, TOKEN_INT_TYPE)) {
        return type_create(TYPE_KIND_INT, "int");
    }
    if (parser_accept(parser, TOKEN_STRING_TYPE)) {
        return type_create(TYPE_KIND_STRING, "string");
    }
    return type_create(TYPE_KIND_VOID, "void");
}

static AstNode *parse_expression(Parser *parser) {
    const Token *token = parser_peek(parser);
    if (token == NULL) {
        return NULL;
    }
    if (token->kind == TOKEN_INT) {
        parser_next(parser);
        return ast_integer_literal_create(strtoll(token->text, NULL, 10), &token->span);
    }
    if (token->kind == TOKEN_STRING) {
        parser_next(parser);
        return ast_string_literal_create(token->text, &token->span);
    }
    if (token->kind == TOKEN_IDENT) {
        parser_next(parser);
        AstNode *identifier = ast_identifier_create(token->text, &token->span);
        if (parser_accept(parser, TOKEN_LPAREN)) {
            AstNode *call = ast_call_create(identifier->as.identifier.name, &identifier->span);
            ast_node_free(identifier);
            if (call == NULL) {
                return NULL;
            }
            while (!parser_accept(parser, TOKEN_RPAREN)) {
                AstNode *arg = parse_expression(parser);
                if (arg == NULL) {
                    ast_node_free(call);
                    return NULL;
                }
                ast_list_push(&call->as.call.arguments, arg);
                parser_accept(parser, TOKEN_COMMA);
            }
            return call;
        }
        return identifier;
    }
    if (token->kind == TOKEN_LPAREN) {
        parser_next(parser);
        AstNode *value = parse_expression(parser);
        if (value == NULL) {
            return NULL;
        }
        parser_expect(parser, TOKEN_RPAREN);
        return value;
    }
    return NULL;
}

static AstNode *parse_statement(Parser *parser) {
    if (parser_accept(parser, TOKEN_PRINT)) {
        const Token *open = parser_expect(parser, TOKEN_LPAREN);
        if (open == NULL) {
            return NULL;
        }
        AstNode *arg = parse_expression(parser);
        if (arg == NULL) {
            return NULL;
        }
        const Token *close = parser_expect(parser, TOKEN_RPAREN);
        if (close == NULL) {
            ast_node_free(arg);
            return NULL;
        }
        parser_accept(parser, TOKEN_SEMICOLON);

        AstNode *node = ast_call_create("print", &open->span);
        if (node == NULL) {
            ast_node_free(arg);
            return NULL;
        }
        ast_list_push(&node->as.call.arguments, arg);
        node->type = type_create(TYPE_KIND_VOID, "void");
        return node;
    }

    if (parser_accept(parser, TOKEN_RETURN)) {
        AstNode *value = parse_expression(parser);
        if (value == NULL) {
            return NULL;
        }
        parser_accept(parser, TOKEN_SEMICOLON);
        const Token *token = parser_peek(parser);
        return ast_return_create(value, token == NULL ? &value->span : &token->span);
    }

    if (parser_accept(parser, TOKEN_LET)) {
        const Token *name = parser_expect(parser, TOKEN_IDENT);
        if (name == NULL) {
            return NULL;
        }
        Type type = type_create(TYPE_KIND_UNKNOWN, "unknown");
        if (parser_accept(parser, TOKEN_COLON)) {
            type = parse_type(parser);
        }
        AstNode *initializer = NULL;
        if (parser_accept(parser, TOKEN_EQUAL)) {
            initializer = parse_expression(parser);
        }
        parser_accept(parser, TOKEN_SEMICOLON);
        return ast_var_decl_create(name->text, &type, initializer, &name->span);
    }

    return NULL;
}

static AstNode *parse_function(Parser *parser) {
    const Token *fn = parser_expect(parser, TOKEN_FN);
    if (fn == NULL) {
        return NULL;
    }
    const Token *name = parser_expect(parser, TOKEN_IDENT);
    if (name == NULL) {
        return NULL;
    }

    const Token *open = parser_expect(parser, TOKEN_LPAREN);
    if (open == NULL) {
        return NULL;
    }

    Type return_type = type_create(TYPE_KIND_VOID, "void");
    AstNode *function = ast_function_create(name->text, &return_type, &name->span);
    if (function == NULL) {
        return NULL;
    }

    if (!parser_accept(parser, TOKEN_RPAREN)) {
        while (1) {
            const Token *param_name = parser_expect(parser, TOKEN_IDENT);
            if (param_name == NULL) {
                ast_node_free(function);
                return NULL;
            }
            parser_expect(parser, TOKEN_COLON);
            Type param_type = parse_type(parser);
            AstNode *parameter = ast_parameter_create(param_name->text, &param_type, &param_name->span);
            type_free(&param_type);
            ast_list_push(&function->as.function.parameters, parameter);
            if (parser_accept(parser, TOKEN_COMMA)) {
                continue;
            }
            if (parser_expect(parser, TOKEN_RPAREN) != NULL) {
                break;
            }
            ast_node_free(function);
            return NULL;
        }
    }

    if (parser_accept(parser, TOKEN_ARROW)) {
        return_type = parse_type(parser);
        function->as.function.return_type = return_type;
    }

    const Token *brace = parser_expect(parser, TOKEN_LBRACE);
    if (brace == NULL) {
        ast_node_free(function);
        return NULL;
    }

    while (!parser_accept(parser, TOKEN_RBRACE)) {
        if (parser_peek(parser)->kind == TOKEN_EOF) {
            ast_node_free(function);
            return NULL;
        }
        AstNode *statement = parse_statement(parser);
        if (statement == NULL) {
            ast_node_free(function);
            return NULL;
        }
        ast_list_push(&function->as.function.body, statement);
    }

    return function;
}

Program *parse_program(const TokenList *tokens) {
    if (tokens == NULL) {
        return NULL;
    }

    Parser parser = { .tokens = tokens, .index = 0 };
    Program *program = program_create();
    if (program == NULL) {
        return NULL;
    }

    while (parser_peek(&parser)->kind != TOKEN_EOF) {
        AstNode *node = parse_function(&parser);
        if (node == NULL) {
            program_free(program);
            return NULL;
        }
        ast_list_push(&program->functions, node);
    }

    return program;
}
