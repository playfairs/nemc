#include "nem/ast.h"

#include <stdlib.h>
#include <string.h>

static AstNode *ast_node_create(AstKind kind, const Span *span) {
    AstNode *node = calloc(1, sizeof(AstNode));
    if (node == NULL) {
        return NULL;
    }
    node->kind = kind;
    node->span = *span;
    node->type = type_create(TYPE_KIND_UNKNOWN, "unknown");
    return node;
}

static void ast_list_init(AstList *list) {
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

Program *program_create(void) {
    Program *program = calloc(1, sizeof(Program));
    if (program == NULL) {
        return NULL;
    }
    ast_list_init(&program->functions);
    return program;
}

void program_free(Program *program) {
    if (program == NULL) {
        return;
    }
    for (size_t i = 0; i < program->functions.count; ++i) {
        ast_node_free(program->functions.items[i]);
    }
    free(program->functions.items);
    free(program);
}

void ast_list_push(AstList *list, AstNode *node) {
    if (node == NULL) {
        return;
    }
    if (list->count == list->capacity) {
        size_t new_capacity = list->capacity == 0 ? 4 : list->capacity * 2;
        AstNode **new_items = realloc(list->items, new_capacity * sizeof(AstNode *));
        if (new_items == NULL) {
            return;
        }
        list->items = new_items;
        list->capacity = new_capacity;
    }
    list->items[list->count++] = node;
}

Type type_create(TypeKind kind, const char *name) {
    Type type;
    type.kind = kind;
    type.name = name == NULL ? NULL : strdup(name);
    return type;
}

Type type_copy(const Type *type) {
    if (type == NULL) {
        return type_create(TYPE_KIND_UNKNOWN, "unknown");
    }
    return type_create(type->kind, type->name == NULL ? "unknown" : type->name);
}

void type_free(Type *type) {
    if (type == NULL) {
        return;
    }
    free(type->name);
    type->name = NULL;
    type->kind = TYPE_KIND_UNKNOWN;
}

AstNode *ast_function_create(const char *name, const Type *return_type, const Span *span) {
    AstNode *node = ast_node_create(AST_FUNCTION, span);
    if (node == NULL) {
        return NULL;
    }
    node->as.function.name = strdup(name);
    Type default_type = type_create(TYPE_KIND_VOID, "void");
    node->as.function.return_type = type_copy(return_type == NULL ? &default_type : return_type);
    type_free(&default_type);
    node->type = type_copy(&node->as.function.return_type);
    ast_list_init(&node->as.function.parameters);
    ast_list_init(&node->as.function.body);
    return node;
}

AstNode *ast_parameter_create(const char *name, const Type *type, const Span *span) {
    AstNode *node = ast_node_create(AST_PARAMETER, span);
    if (node == NULL) {
        return NULL;
    }
    node->as.parameter.name = strdup(name);
    Type default_type = type_create(TYPE_KIND_UNKNOWN, "unknown");
    node->as.parameter.type = type_copy(type == NULL ? &default_type : type);
    node->type = type_copy(&node->as.parameter.type);
    type_free(&default_type);
    return node;
}

AstNode *ast_var_decl_create(const char *name, const Type *type, int is_mutable, AstNode *initializer, const Span *span) {
    AstNode *node = ast_node_create(AST_VAR_DECL, span);
    if (node == NULL) {
        return NULL;
    }
    node->as.var_decl.name = strdup(name);
    Type default_type = type_create(TYPE_KIND_UNKNOWN, "unknown");
    node->as.var_decl.type = type_copy(type == NULL ? &default_type : type);
    node->as.var_decl.is_mutable = is_mutable;
    node->type = type_copy(&node->as.var_decl.type);
    node->as.var_decl.initializer = initializer;
    type_free(&default_type);
    return node;
}

AstNode *ast_block_create(const Span *span) {
    AstNode *node = ast_node_create(AST_BLOCK, span);
    if (node == NULL) {
        return NULL;
    }
    ast_list_init(&node->as.block.statements);
    return node;
}

AstNode *ast_return_create(AstNode *value, const Span *span) {
    AstNode *node = ast_node_create(AST_RETURN, span);
    if (node == NULL) {
        return NULL;
    }
    node->as.return_stmt.value = value;
    return node;
}

AstNode *ast_integer_literal_create(long long value, const Span *span) {
    AstNode *node = ast_node_create(AST_INTEGER_LITERAL, span);
    if (node == NULL) {
        return NULL;
    }
    node->as.integer_literal.value = value;
    node->type = type_create(TYPE_KIND_INT, "int");
    return node;
}

AstNode *ast_string_literal_create(const char *text, const Span *span) {
    AstNode *node = ast_node_create(AST_STRING_LITERAL, span);
    if (node == NULL) {
        return NULL;
    }
    node->as.string_literal.text = strdup(text);
    node->type = type_create(TYPE_KIND_STRING, "string");
    return node;
}

AstNode *ast_bool_literal_create(int value, const Span *span) {
    AstNode *node = ast_node_create(AST_BOOL_LITERAL, span);
    if (node == NULL) {
        return NULL;
    }
    node->as.bool_literal.value = value;
    node->type = type_create(TYPE_KIND_BOOL, "bool");
    return node;
}

AstNode *ast_identifier_create(const char *name, const Span *span) {
    AstNode *node = ast_node_create(AST_IDENTIFIER, span);
    if (node == NULL) {
        return NULL;
    }
    node->as.identifier.name = strdup(name);
    return node;
}

AstNode *ast_call_create(const char *name, const Span *span) {
    AstNode *node = ast_node_create(AST_CALL, span);
    if (node == NULL) {
        return NULL;
    }
    node->as.call.name = strdup(name);
    ast_list_init(&node->as.call.arguments);
    return node;
}

AstNode *ast_binary_expr_create(AstNode *left, char op, AstNode *right, const Span *span) {
    AstNode *node = ast_node_create(AST_BINARY_EXPR, span);
    if (node == NULL) {
        return NULL;
    }
    node->as.binary_expr.left = left;
    node->as.binary_expr.right = right;
    node->as.binary_expr.op = op;
    return node;
}

AstNode *ast_unary_expr_create(char op, AstNode *value, const Span *span) {
    AstNode *node = ast_node_create(AST_UNARY_EXPR, span);
    if (node == NULL) {
        return NULL;
    }
    node->as.unary_expr.op = op;
    node->as.unary_expr.value = value;
    return node;
}

AstNode *ast_assignment_create(AstNode *target, AstNode *value, const Span *span) {
    AstNode *node = ast_node_create(AST_ASSIGNMENT, span);
    if (node == NULL) {
        return NULL;
    }
    node->as.assignment.target = target;
    node->as.assignment.value = value;
    return node;
}

AstNode *ast_if_create(AstNode *condition, AstNode *then_branch, AstNode *else_branch, const Span *span) {
    AstNode *node = ast_node_create(AST_IF, span);
    if (node == NULL) {
        return NULL;
    }
    node->as.if_stmt.condition = condition;
    node->as.if_stmt.then_branch = then_branch;
    node->as.if_stmt.else_branch = else_branch;
    return node;
}

AstNode *ast_while_create(AstNode *condition, AstNode *body, const Span *span) {
    AstNode *node = ast_node_create(AST_WHILE, span);
    if (node == NULL) {
        return NULL;
    }
    node->as.while_stmt.condition = condition;
    node->as.while_stmt.body = body;
    return node;
}

void ast_node_free(AstNode *node) {
    if (node == NULL) {
        return;
    }
    if (node->kind == AST_FUNCTION) {
        free(node->as.function.name);
        type_free(&node->as.function.return_type);
        for (size_t i = 0; i < node->as.function.parameters.count; ++i) {
            ast_node_free(node->as.function.parameters.items[i]);
        }
        free(node->as.function.parameters.items);
        for (size_t i = 0; i < node->as.function.body.count; ++i) {
            ast_node_free(node->as.function.body.items[i]);
        }
        free(node->as.function.body.items);
    } else if (node->kind == AST_PARAMETER) {
        free(node->as.parameter.name);
        type_free(&node->as.parameter.type);
    } else if (node->kind == AST_VAR_DECL) {
        free(node->as.var_decl.name);
        type_free(&node->as.var_decl.type);
        ast_node_free(node->as.var_decl.initializer);
    } else if (node->kind == AST_BLOCK) {
        for (size_t i = 0; i < node->as.block.statements.count; ++i) {
            ast_node_free(node->as.block.statements.items[i]);
        }
        free(node->as.block.statements.items);
    } else if (node->kind == AST_RETURN) {
        ast_node_free(node->as.return_stmt.value);
    } else if (node->kind == AST_STRING_LITERAL) {
        free(node->as.string_literal.text);
    } else if (node->kind == AST_BOOL_LITERAL) {
        /* boolean literals hold no heap state */
    } else if (node->kind == AST_IDENTIFIER) {
        free(node->as.identifier.name);
    } else if (node->kind == AST_CALL) {
        free(node->as.call.name);
        for (size_t i = 0; i < node->as.call.arguments.count; ++i) {
            ast_node_free(node->as.call.arguments.items[i]);
        }
        free(node->as.call.arguments.items);
    } else if (node->kind == AST_BINARY_EXPR) {
        ast_node_free(node->as.binary_expr.left);
        ast_node_free(node->as.binary_expr.right);
    } else if (node->kind == AST_UNARY_EXPR) {
        ast_node_free(node->as.unary_expr.value);
    } else if (node->kind == AST_ASSIGNMENT) {
        ast_node_free(node->as.assignment.target);
        ast_node_free(node->as.assignment.value);
    } else if (node->kind == AST_IF) {
        ast_node_free(node->as.if_stmt.condition);
        ast_node_free(node->as.if_stmt.then_branch);
        ast_node_free(node->as.if_stmt.else_branch);
    } else if (node->kind == AST_WHILE) {
        ast_node_free(node->as.while_stmt.condition);
        ast_node_free(node->as.while_stmt.body);
    }
    type_free(&node->type);
    free(node);
}
