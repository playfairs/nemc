#include "nem/ir.h"

#include <stdlib.h>
#include <string.h>

static void ir_list_init(IrList *list) {
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

static void ir_list_push(IrList *list, IrNode *node) {
    if (node == NULL) {
        return;
    }
    if (list->count == list->capacity) {
        size_t new_capacity = list->capacity == 0 ? 4 : list->capacity * 2;
        IrNode **new_items = realloc(list->items, new_capacity * sizeof(IrNode *));
        if (new_items == NULL) {
            return;
        }
        list->items = new_items;
        list->capacity = new_capacity;
    }
    list->items[list->count++] = node;
}

IrNode *ir_function_create(const char *name, const Type *return_type) {
    IrNode *node = calloc(1, sizeof(IrNode));
    if (node == NULL) {
        return NULL;
    }
    node->kind = IR_FUNCTION;
    node->as.function.name = strdup(name == NULL ? "" : name);
    node->as.function.return_type = type_copy(return_type == NULL ? &(Type){TYPE_KIND_VOID, "void"} : return_type);
    node->type = type_copy(&node->as.function.return_type);
    ir_list_init(&node->as.function.body);
    return node;
}

IrNode *ir_parameter_create(const char *name, const Type *type) {
    IrNode *node = calloc(1, sizeof(IrNode));
    if (node == NULL) {
        return NULL;
    }
    node->kind = IR_PARAMETER;
    node->as.parameter.name = strdup(name == NULL ? "" : name);
    node->type = type_copy(type == NULL ? &(Type){TYPE_KIND_UNKNOWN, "unknown"} : type);
    return node;
}

IrNode *ir_var_create(const char *name, const Type *type) {
    IrNode *node = calloc(1, sizeof(IrNode));
    if (node == NULL) {
        return NULL;
    }
    node->kind = IR_VAR;
    node->as.var.name = strdup(name == NULL ? "" : name);
    node->type = type_copy(type == NULL ? &(Type){TYPE_KIND_UNKNOWN, "unknown"} : type);
    return node;
}

IrNode *ir_constant_create(long long value, const Type *type) {
    IrNode *node = calloc(1, sizeof(IrNode));
    if (node == NULL) {
        return NULL;
    }
    node->kind = IR_CONSTANT;
    node->as.constant.value = value;
    node->type = type_copy(type == NULL ? &(Type){TYPE_KIND_INT, "int"} : type);
    return node;
}

IrNode *ir_string_create(const char *text) {
    IrNode *node = calloc(1, sizeof(IrNode));
    if (node == NULL) {
        return NULL;
    }
    node->kind = IR_STRING;
    node->as.string_literal.text = strdup(text == NULL ? "" : text);
    node->type = type_create(TYPE_KIND_STRING, "string");
    return node;
}

IrNode *ir_bool_create(int value) {
    IrNode *node = calloc(1, sizeof(IrNode));
    if (node == NULL) {
        return NULL;
    }
    node->kind = IR_BOOL;
    node->as.bool_literal.value = value;
    node->type = type_create(TYPE_KIND_BOOL, "bool");
    return node;
}

IrNode *ir_binary_create(IrNode *left, char op, IrNode *right, const Type *type) {
    IrNode *node = calloc(1, sizeof(IrNode));
    if (node == NULL) {
        return NULL;
    }
    node->kind = IR_BINARY;
    node->as.binary.left = left;
    node->as.binary.right = right;
    node->as.binary.op = op;
    node->type = type_copy(type == NULL ? &(Type){TYPE_KIND_INT, "int"} : type);
    return node;
}

IrNode *ir_unary_create(char op, IrNode *value, const Type *type) {
    IrNode *node = calloc(1, sizeof(IrNode));
    if (node == NULL) {
        return NULL;
    }
    node->kind = IR_UNARY;
    node->as.unary.op = op;
    node->as.unary.value = value;
    node->type = type_copy(type == NULL ? &(Type){TYPE_KIND_BOOL, "bool"} : type);
    return node;
}

IrNode *ir_call_create(const char *name, const Type *type) {
    IrNode *node = calloc(1, sizeof(IrNode));
    if (node == NULL) {
        return NULL;
    }
    node->kind = IR_CALL;
    node->as.call.name = strdup(name == NULL ? "" : name);
    ir_list_init(&node->as.call.arguments);
    node->type = type_copy(type == NULL ? &(Type){TYPE_KIND_VOID, "void"} : type);
    return node;
}

IrNode *ir_return_create(IrNode *value) {
    IrNode *node = calloc(1, sizeof(IrNode));
    if (node == NULL) {
        return NULL;
    }
    node->kind = IR_RETURN;
    node->as.return_stmt.value = value;
    node->type = value == NULL ? type_create(TYPE_KIND_VOID, "void") : type_copy(&value->type);
    return node;
}

IrNode *ir_decl_create(const char *name, const Type *type, IrNode *initializer) {
    IrNode *node = calloc(1, sizeof(IrNode));
    if (node == NULL) {
        return NULL;
    }
    node->kind = IR_DECL;
    node->as.decl.name = strdup(name == NULL ? "" : name);
    node->as.decl.type = type_copy(type == NULL ? &(Type){TYPE_KIND_UNKNOWN, "unknown"} : type);
    node->as.decl.initializer = initializer;
    node->type = type_copy(type == NULL ? &(Type){TYPE_KIND_UNKNOWN, "unknown"} : type);
    return node;
}

IrNode *ir_assign_create(const char *name, IrNode *value) {
    IrNode *node = calloc(1, sizeof(IrNode));
    if (node == NULL) {
        return NULL;
    }
    node->kind = IR_ASSIGN;
    node->as.assign.name = strdup(name == NULL ? "" : name);
    node->as.assign.value = value;
    node->type = value == NULL ? type_create(TYPE_KIND_VOID, "void") : type_copy(&value->type);
    return node;
}

IrNode *ir_if_create(IrNode *condition, IrNode *then_branch, IrNode *else_branch) {
    IrNode *node = calloc(1, sizeof(IrNode));
    if (node == NULL) {
        return NULL;
    }
    node->kind = IR_IF;
    node->as.if_stmt.condition = condition;
    node->as.if_stmt.then_branch = then_branch;
    node->as.if_stmt.else_branch = else_branch;
    node->type = type_create(TYPE_KIND_VOID, "void");
    return node;
}

IrNode *ir_while_create(IrNode *condition, IrNode *body) {
    IrNode *node = calloc(1, sizeof(IrNode));
    if (node == NULL) {
        return NULL;
    }
    node->kind = IR_WHILE;
    node->as.while_stmt.condition = condition;
    node->as.while_stmt.body = body;
    node->type = type_create(TYPE_KIND_VOID, "void");
    return node;
}

IrNode *ir_block_create(IrList *statements) {
    IrNode *node = calloc(1, sizeof(IrNode));
    if (node == NULL) {
        return NULL;
    }
    node->kind = IR_BLOCK;
    ir_list_init(&node->as.block.statements);
    if (statements != NULL) {
        for (size_t i = 0; i < statements->count; ++i) {
            ir_list_append(&node->as.block.statements, statements->items[i]);
        }
    }
    node->type = type_create(TYPE_KIND_VOID, "void");
    return node;
}

void ir_list_append(IrList *list, IrNode *node) {
    ir_list_push(list, node);
}

void ir_node_free(IrNode *node) {
    if (node == NULL) {
        return;
    }
    if (node->kind == IR_FUNCTION) {
        free(node->as.function.name);
        type_free(&node->as.function.return_type);
        for (size_t i = 0; i < node->as.function.body.count; ++i) {
            ir_node_free(node->as.function.body.items[i]);
        }
        free(node->as.function.body.items);
    } else if (node->kind == IR_PARAMETER) {
        free(node->as.parameter.name);
    } else if (node->kind == IR_VAR) {
        free(node->as.var.name);
    } else if (node->kind == IR_CALL) {
        free(node->as.call.name);
        for (size_t i = 0; i < node->as.call.arguments.count; ++i) {
            ir_node_free(node->as.call.arguments.items[i]);
        }
        free(node->as.call.arguments.items);
    } else if (node->kind == IR_STRING) {
        free(node->as.string_literal.text);
    } else if (node->kind == IR_BINARY) {
        ir_node_free(node->as.binary.left);
        ir_node_free(node->as.binary.right);
    } else if (node->kind == IR_UNARY) {
        ir_node_free(node->as.unary.value);
    } else if (node->kind == IR_RETURN) {
        ir_node_free(node->as.return_stmt.value);
    } else if (node->kind == IR_DECL) {
        free(node->as.decl.name);
        type_free(&node->as.decl.type);
        ir_node_free(node->as.decl.initializer);
    } else if (node->kind == IR_ASSIGN) {
        free(node->as.assign.name);
        ir_node_free(node->as.assign.value);
    } else if (node->kind == IR_IF) {
        ir_node_free(node->as.if_stmt.condition);
        ir_node_free(node->as.if_stmt.then_branch);
        ir_node_free(node->as.if_stmt.else_branch);
    } else if (node->kind == IR_WHILE) {
        ir_node_free(node->as.while_stmt.condition);
        ir_node_free(node->as.while_stmt.body);
    } else if (node->kind == IR_BLOCK) {
        for (size_t i = 0; i < node->as.block.statements.count; ++i) {
            ir_node_free(node->as.block.statements.items[i]);
        }
        free(node->as.block.statements.items);
    }
    type_free(&node->type);
    free(node);
}
