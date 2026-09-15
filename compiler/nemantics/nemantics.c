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

static int is_bool_operation(char op) {
    return op == '&' || op == '|' || op == '!';
}

static int is_comparison_operation(char op) {
    return op == '=' || op == '!' || op == '<' || op == 'L' || op == '>' || op == 'G';
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
        case AST_BOOL_LITERAL:
            return type_create(TYPE_KIND_BOOL, "bool");
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
        case AST_BINARY_EXPR: {
            Type left = infer_expression_type(node->as.binary_expr.left, scope);
            Type right = infer_expression_type(node->as.binary_expr.right, scope);
            Type result = type_create(TYPE_KIND_UNKNOWN, "unknown");
            if (is_bool_operation(node->as.binary_expr.op)) {
                if (type_equal(&left, &(Type){TYPE_KIND_BOOL, "bool"}) && type_equal(&right, &(Type){TYPE_KIND_BOOL, "bool"})) {
                    result = type_create(TYPE_KIND_BOOL, "bool");
                }
            } else if (is_comparison_operation(node->as.binary_expr.op)) {
                if (type_equal(&left, &(Type){TYPE_KIND_INT, "int"}) && type_equal(&right, &(Type){TYPE_KIND_INT, "int"})) {
                    result = type_create(TYPE_KIND_BOOL, "bool");
                }
            } else {
                if (type_equal(&left, &(Type){TYPE_KIND_INT, "int"}) && type_equal(&right, &(Type){TYPE_KIND_INT, "int"})) {
                    result = type_create(TYPE_KIND_INT, "int");
                }
            }
            type_free(&left);
            type_free(&right);
            return result;
        }
        case AST_UNARY_EXPR: {
            Type value = infer_expression_type(node->as.unary_expr.value, scope);
            Type result = type_create(TYPE_KIND_UNKNOWN, "unknown");
            if (node->as.unary_expr.op == '!') {
                if (type_equal(&value, &(Type){TYPE_KIND_BOOL, "bool"})) {
                    result = type_create(TYPE_KIND_BOOL, "bool");
                }
            }
            type_free(&value);
            return result;
        }
        default:
            return type_create(TYPE_KIND_UNKNOWN, "unknown");
    }
}

static int block_returns_on_all_paths(const AstList *block) {
    if (block == NULL) {
        return 0;
    }
    for (size_t i = 0; i < block->count; ++i) {
        const AstNode *statement = block->items[i];
        if (statement == NULL) {
            continue;
        }
        if (statement->kind == AST_RETURN) {
            return 1;
        }
        if (statement->kind == AST_IF) {
            if (statement->as.if_stmt.then_branch == NULL || statement->as.if_stmt.else_branch == NULL) {
                continue;
            }
            if (statement->as.if_stmt.then_branch->kind == AST_BLOCK && statement->as.if_stmt.else_branch->kind == AST_BLOCK) {
                if (block_returns_on_all_paths(&statement->as.if_stmt.then_branch->as.block.statements) &&
                    block_returns_on_all_paths(&statement->as.if_stmt.else_branch->as.block.statements)) {
                    return 1;
                }
            }
        }
    }
    return 0;
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

void scope_define_symbol(Scope *scope, const char *name, const Type *type, int is_parameter, int is_function, int is_mutable) {
    if (scope == NULL || name == NULL) {
        return;
    }
    Symbol symbol;
    symbol.name = strdup(name);
    symbol.type = type == NULL ? type_create(TYPE_KIND_UNKNOWN, "unknown") : type_copy(type);
    symbol.is_parameter = is_parameter;
    symbol.is_function = is_function;
    symbol.is_defined = 1;
    symbol.is_mutable = is_mutable;
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
        scope_define_symbol(global, function->as.function.name, &fn_type, 0, 1, 0);

        Scope *local = NULL;
        scope_push(global, &local);
        for (size_t j = 0; j < function->as.function.parameters.count; ++j) {
            const AstNode *parameter = function->as.function.parameters.items[j];
            scope_define_symbol(local, parameter->as.parameter.name, &parameter->as.parameter.type, 1, 0, 0);
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
                scope_define_symbol(local, statement->as.var_decl.name, &statement->as.var_decl.type, 0, 0, statement->as.var_decl.is_mutable);
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
            } else if (statement->kind == AST_ASSIGNMENT) {
                if (statement->as.assignment.target == NULL || statement->as.assignment.target->kind != AST_IDENTIFIER) {
                    diagnostics_add(diagnostics, DIAGNOSTIC_ERROR, "invalid assignment target", &statement->span);
                    continue;
                }
                Symbol *target = scope_lookup(local, statement->as.assignment.target->as.identifier.name);
                if (target == NULL) {
                    diagnostics_add(diagnostics, DIAGNOSTIC_ERROR, "undefined variable in assignment", &statement->span);
                    continue;
                }
                if (!target->is_mutable) {
                    diagnostics_add(diagnostics, DIAGNOSTIC_ERROR, "cannot assign to immutable variable", &statement->span);
                }
                Type value_type = infer_expression_type(statement->as.assignment.value, local);
                if (!type_equal(&target->type, &value_type)) {
                    diagnostics_add(diagnostics, DIAGNOSTIC_ERROR, "assignment type mismatch", &statement->span);
                }
                type_free(&value_type);
            } else if (statement->kind == AST_IF) {
                Type cond = infer_expression_type(statement->as.if_stmt.condition, local);
                if (!type_equal(&cond, &(Type){TYPE_KIND_BOOL, "bool"})) {
                    diagnostics_add(diagnostics, DIAGNOSTIC_ERROR, "expected bool condition", &statement->span);
                }
                type_free(&cond);
            } else if (statement->kind == AST_WHILE) {
                Type cond = infer_expression_type(statement->as.while_stmt.condition, local);
                if (!type_equal(&cond, &(Type){TYPE_KIND_BOOL, "bool"})) {
                    diagnostics_add(diagnostics, DIAGNOSTIC_ERROR, "expected bool condition", &statement->span);
                }
                type_free(&cond);
            }
        }

        if (function->as.function.return_type.kind != TYPE_KIND_VOID && !block_returns_on_all_paths(&function->as.function.body)) {
            diagnostics_add(diagnostics, DIAGNOSTIC_ERROR, "missing return value on some control-flow paths", &function->span);
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
