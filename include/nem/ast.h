#ifndef NEM_AST_H
#define NEM_AST_H

#include <stddef.h>

#include "nem/source.h"

typedef enum {
    TYPE_KIND_VOID,
    TYPE_KIND_INT,
    TYPE_KIND_STRING,
    TYPE_KIND_BOOL,
    TYPE_KIND_UNKNOWN
} TypeKind;

typedef struct {
    TypeKind kind;
    char *name;
} Type;

typedef enum {
    AST_FUNCTION,
    AST_PARAMETER,
    AST_VAR_DECL,
    AST_BLOCK,
    AST_RETURN,
    AST_INTEGER_LITERAL,
    AST_STRING_LITERAL,
    AST_IDENTIFIER,
    AST_CALL,
    AST_BINARY_EXPR
} AstKind;

typedef struct AstNode AstNode;

typedef struct {
    AstNode **items;
    size_t count;
    size_t capacity;
} AstList;

struct AstNode {
    AstKind kind;
    Span span;
    Type type;
    union {
        struct {
            char *name;
            Type return_type;
            AstList parameters;
            AstList body;
        } function;
        struct {
            char *name;
            Type type;
        } parameter;
        struct {
            char *name;
            Type type;
            AstNode *initializer;
        } var_decl;
        struct {
            AstList statements;
        } block;
        struct {
            AstNode *value;
        } return_stmt;
        struct {
            long long value;
        } integer_literal;
        struct {
            char *text;
        } string_literal;
        struct {
            char *name;
        } identifier;
        struct {
            char *name;
            AstList arguments;
        } call;
        struct {
            AstNode *left;
            AstNode *right;
            char op;
        } binary_expr;
    } as;
};

typedef struct {
    AstList functions;
} Program;

Program *program_create(void);
void program_free(Program *program);
AstNode *ast_function_create(const char *name, const Type *return_type, const Span *span);
AstNode *ast_parameter_create(const char *name, const Type *type, const Span *span);
AstNode *ast_var_decl_create(const char *name, const Type *type, AstNode *initializer, const Span *span);
AstNode *ast_block_create(const Span *span);
AstNode *ast_return_create(AstNode *value, const Span *span);
AstNode *ast_integer_literal_create(long long value, const Span *span);
AstNode *ast_string_literal_create(const char *text, const Span *span);
AstNode *ast_identifier_create(const char *name, const Span *span);
AstNode *ast_call_create(const char *name, const Span *span);
AstNode *ast_binary_expr_create(AstNode *left, char op, AstNode *right, const Span *span);
void ast_list_push(AstList *list, AstNode *node);
void ast_node_free(AstNode *node);
Type type_create(TypeKind kind, const char *name);
Type type_copy(const Type *type);
void type_free(Type *type);

#endif
