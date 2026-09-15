#ifndef NEM_IR_H
#define NEM_IR_H

#include <stddef.h>

#include "nem/ast.h"

typedef enum {
    IR_FUNCTION,
    IR_PARAMETER,
    IR_VAR,
    IR_CONSTANT,
    IR_STRING,
    IR_BOOL,
    IR_BINARY,
    IR_UNARY,
    IR_CALL,
    IR_RETURN,
    IR_DECL,
    IR_ASSIGN,
    IR_IF,
    IR_WHILE,
    IR_BLOCK
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
            int value;
        } bool_literal;
        struct {
            long long value;
        } constant;
        struct {
            IrNode *left;
            IrNode *right;
            char op;
        } binary;
        struct {
            char op;
            IrNode *value;
        } unary;
        struct {
            char *name;
            IrList arguments;
        } call;
        struct {
            IrNode *value;
        } return_stmt;
        struct {
            char *name;
            Type type;
            IrNode *initializer;
        } decl;
        struct {
            char *name;
            IrNode *value;
        } assign;
        struct {
            IrNode *condition;
            IrNode *then_branch;
            IrNode *else_branch;
        } if_stmt;
        struct {
            IrNode *condition;
            IrNode *body;
        } while_stmt;
        struct {
            IrList statements;
        } block;
    } as;
};

IrNode *ir_function_create(const char *name, const Type *return_type);
IrNode *ir_parameter_create(const char *name, const Type *type);
IrNode *ir_var_create(const char *name, const Type *type);
IrNode *ir_constant_create(long long value, const Type *type);
IrNode *ir_string_create(const char *text);
IrNode *ir_bool_create(int value);
IrNode *ir_binary_create(IrNode *left, char op, IrNode *right, const Type *type);
IrNode *ir_unary_create(char op, IrNode *value, const Type *type);
IrNode *ir_call_create(const char *name, const Type *type);
IrNode *ir_return_create(IrNode *value);
IrNode *ir_decl_create(const char *name, const Type *type, IrNode *initializer);
IrNode *ir_assign_create(const char *name, IrNode *value);
IrNode *ir_if_create(IrNode *condition, IrNode *then_branch, IrNode *else_branch);
IrNode *ir_while_create(IrNode *condition, IrNode *body);
IrNode *ir_block_create(IrList *statements);
void ir_list_append(IrList *list, IrNode *node);
void ir_node_free(IrNode *node);

#endif
