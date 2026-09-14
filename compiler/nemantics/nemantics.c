#include "nem/nemantics.h"

#include <stdlib.h>
#include <string.h>

static void symbol_table_init(SymbolTable *table) {
    table->items = NULL;
    table->count = 0;
    table->capacity = 0;
}

static void symbol_table_push(SymbolTable *table, const Symbol *symbol) {
    if (table == NULL || symbol == NULL) {
        return;
    }
    if (table->count == table->capacity) {
        size_t new_capacity = table->capacity == 0 ? 8 : table->capacity * 2;
        Symbol *items = realloc(table->items, new_capacity * sizeof(Symbol));
        if (items == NULL) {
            return;
        }
        table->items = items;
        table->capacity = new_capacity;
    }
    table->items[table->count++] = *symbol;
}

static int type_equal(const Type *left, const Type *right) {
    if (left == NULL || right == NULL) {
        return left == right;
    }
    if (left->kind != right->kind) {
        return 0;
    }
    if (left->name == NULL || right->name == NULL) {
        return left->name == right->name;
    }
    return strcmp(left->name, right->name) == 0;
}

static Type infer_expression_type(const AstNode *node, Scope *scope) {
    if (node == NULL) {
        return type_create(TYPE_KIND_UNKNOWN, "unknown");
    }
    switch (node->kind) {
        case AST_INTEGER_LITERAL:
            return type_create(TYPE_KIND_INT, "int");
        case AST_STRING_LITERAL:
            return type_create(TYPE_KIND_STRING, "string");
        case AST_IDENTIFIER: {
            if (scope == NULL) {
                return type_create(TYPE_KIND_UNKNOWN, "unknown");
            }
            Symbol *symbol = scope_lookup(scope, node->as.identifier.name);
            if (symbol == NULL) {
                return type_create(TYPE_KIND_UNKNOWN, "unknown");
            }
            return symbol->type;
        }
        case AST_CALL:
            return node->type;
        case AST_BINARY_EXPR:
            return type_create(TYPE_KIND_INT, "int");
        default:
            return type_create(TYPE_KIND_UNKNOWN, "unknown");
    }
}

void scope_push(Scope *scope, Scope **current) {
    if (current == NULL) {
        return;
    }
    Scope *next = calloc(1, sizeof(Scope));
    if (next == NULL) {
        return;
    }
    next->name = scope == NULL ? "local" : scope->name;
    next->parent = *current;
    symbol_table_init(&next->symbols);
    *current = next;
}

void scope_pop(Scope **current) {
    if (current == NULL || *current == NULL) {
        return;
    }
    Scope *next = (*current)->parent;
    for (size_t i = 0; i < (*current)->symbols.count; ++i) {
        free((*current)->symbols.items[i].name);
    }
    free((*current)->symbols.items);
    free(*current);
    *current = next;
}

void scope_define_symbol(Scope *scope, const char *name, const Type *type, int is_parameter, int is_function) {
    if (scope == NULL || name == NULL) {
        return;
    }
    Symbol symbol;
    symbol.name = strdup(name);
    symbol.type = type == NULL ? type_create(TYPE_KIND_UNKNOWN, "unknown") : type_copy(type);
    symbol.is_parameter = is_parameter;
    symbol.is_function = is_function;
    symbol.is_defined = 1;
    symbol_table_push(&scope->symbols, &symbol);
}

Symbol *scope_lookup(Scope *scope, const char *name) {
    if (scope == NULL || name == NULL) {
        return NULL;
    }
    for (Scope *current = scope; current != NULL; current = current->parent) {
        for (size_t i = 0; i < current->symbols.count; ++i) {
            if (strcmp(current->symbols.items[i].name, name) == 0) {
                return &current->symbols.items[i];
            }
        }
    }
    return NULL;
}

Diagnostics *nemantics_validate(const Program *program) {
    Diagnostics *diagnostics = diagnostics_create();
    if (program == NULL) {
        return diagnostics;
    }

    Scope *global = calloc(1, sizeof(Scope));
    if (global == NULL) {
        return diagnostics;
    }
    global->name = "global";
    symbol_table_init(&global->symbols);

    for (size_t i = 0; i < program->functions.count; ++i) {
        const AstNode *function = program->functions.items[i];
        if (function == NULL || function->kind != AST_FUNCTION) {
            diagnostics_add(diagnostics, DIAGNOSTIC_ERROR, "invalid function declaration", &function->span);
            continue;
        }
        if (function->as.function.name == NULL || function->as.function.name[0] == '\0') {
            diagnostics_add(diagnostics, DIAGNOSTIC_ERROR, "function name is required", &function->span);
            continue;
        }

        Type fn_type = function->as.function.return_type;
        scope_define_symbol(global, function->as.function.name, &fn_type, 0, 1);

        Scope *local = NULL;
        scope_push(global, &local);
        for (size_t j = 0; j < function->as.function.parameters.count; ++j) {
            const AstNode *parameter = function->as.function.parameters.items[j];
            scope_define_symbol(local, parameter->as.parameter.name, &parameter->as.parameter.type, 1, 0);
        }

        for (size_t j = 0; j < function->as.function.body.count; ++j) {
            const AstNode *statement = function->as.function.body.items[j];
            if (statement == NULL) {
                diagnostics_add(diagnostics, DIAGNOSTIC_ERROR, "null statement", &function->span);
                continue;
            }
            if (statement->kind == AST_VAR_DECL) {
                Type inferred = infer_expression_type(statement->as.var_decl.initializer, local);
                if (statement->as.var_decl.initializer != NULL && !type_equal(&statement->as.var_decl.type, &inferred)) {
                    diagnostics_add(diagnostics, DIAGNOSTIC_ERROR, "variable initializer type mismatch", &statement->span);
                }
                type_free(&inferred);
                scope_define_symbol(local, statement->as.var_decl.name, &statement->as.var_decl.type, 0, 0);
            } else if (statement->kind == AST_RETURN) {
                Type inferred = infer_expression_type(statement->as.return_stmt.value, local);
                if (!type_equal(&function->as.function.return_type, &inferred)) {
                    diagnostics_add(diagnostics, DIAGNOSTIC_ERROR, "return type mismatch", &statement->span);
                }
                type_free(&inferred);
            } else if (statement->kind == AST_CALL && strcmp(statement->as.call.name, "print") == 0) {
                if (statement->as.call.arguments.count > 0) {
                    Type arg_type = infer_expression_type(statement->as.call.arguments.items[0], local);
                    type_free(&arg_type);
                }
            }
        }
        scope_pop(&local);
    }

    for (size_t i = 0; i < global->symbols.count; ++i) {
        free(global->symbols.items[i].name);
    }
    free(global->symbols.items);
    free(global);

    return diagnostics;
}
