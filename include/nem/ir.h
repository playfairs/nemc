#ifndef NEM_IR_H
#define NEM_IR_H

#include <stddef.h>

#include "nem/ast.h"

typedef enum {
    IR_FUNCTION,
    IR_PARAMETER,
    IR_VAR,
    IR_CONSTANT,
    IR_BINARY,
    IR_CALL,
    IR_RETURN,
    IR_STRING
} IrKind;

typedef struct IrNode IrNode;

typedef struct {
    IrNode **items;
    size_t count;
    size_t capacity;
} IrList;

struct IrNode {
    IrKind kind;
    Type type;
    union {
        struct {
            char *name;
            Type return_type;
            IrList parameters;
            IrList body;
        } function;
        struct {
            char *name;
            Type type;
        } parameter;
        struct {
            char *name;
            Type type;
        } var;
        struct {
            char *text;
        } string_literal;
        struct {
            long long value;
        } constant;
        struct {
            IrNode *left;
            IrNode *right;
            char op;
        } binary;
        struct {
            char *name;
            IrList arguments;
        } call;
        struct {
            IrNode *value;
        } return_stmt;
    } as;
};

IrNode *ir_function_create(const char *name, const Type *return_type);
IrNode *ir_parameter_create(const char *name, const Type *type);
IrNode *ir_var_create(const char *name, const Type *type);
IrNode *ir_constant_create(long long value, const Type *type);
IrNode *ir_string_create(const char *text);
IrNode *ir_binary_create(IrNode *left, char op, IrNode *right, const Type *type);
IrNode *ir_call_create(const char *name, const Type *type);
IrNode *ir_return_create(IrNode *value);
void ir_list_append(IrList *list, IrNode *node);
void ir_node_free(IrNode *node);

#endif
