#include "nem/ast.h"
#include "nem/codegen.h"
#include "nem/diagnostics.h"
#include "nem/lexer.h"
#include "nem/nemantics.h"
#include "nem/parser.h"
#include "nem/source.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static IrNode *lower_block(const AstNode *block);

static IrNode *lower_expression(const AstNode *node) {
    if (node == NULL) {
        return NULL;
    }
    switch (node->kind) {
        case AST_INTEGER_LITERAL:
            return ir_constant_create(node->as.integer_literal.value, &node->type);
        case AST_STRING_LITERAL:
            return ir_string_create(node->as.string_literal.text);
        case AST_BOOL_LITERAL:
            return ir_bool_create(node->as.bool_literal.value);
        case AST_IDENTIFIER:
            return ir_var_create(node->as.identifier.name, &node->type);
        case AST_CALL: {
            IrNode *call = ir_call_create(node->as.call.name, &node->type);
            if (call == NULL) {
                return NULL;
            }
            for (size_t i = 0; i < node->as.call.arguments.count; ++i) {
                IrNode *arg = lower_expression(node->as.call.arguments.items[i]);
                if (arg == NULL) {
                    ir_node_free(call);
                    return NULL;
                }
                ir_list_append(&call->as.call.arguments, arg);
            }
            return call;
        }
        case AST_BINARY_EXPR: {
            IrNode *left = lower_expression(node->as.binary_expr.left);
            IrNode *right = lower_expression(node->as.binary_expr.right);
            if (left == NULL || right == NULL) {
                ir_node_free(left);
                ir_node_free(right);
                return NULL;
            }
            return ir_binary_create(left, node->as.binary_expr.op, right, &node->type);
        }
        case AST_UNARY_EXPR: {
            IrNode *value = lower_expression(node->as.unary_expr.value);
            if (value == NULL) {
                return NULL;
            }
            return ir_unary_create(node->as.unary_expr.op, value, &node->type);
        }
        default:
            return NULL;
    }
}

static IrNode *lower_statement(const AstNode *statement) {
    if (statement == NULL) {
        return NULL;
    }
    if (statement->kind == AST_CALL) {
        return lower_expression(statement);
    }
    if (statement->kind == AST_RETURN) {
        IrNode *value = lower_expression(statement->as.return_stmt.value);
        return ir_return_create(value);
    }
    if (statement->kind == AST_VAR_DECL) {
        IrNode *initializer = statement->as.var_decl.initializer == NULL ? NULL : lower_expression(statement->as.var_decl.initializer);
        return ir_decl_create(statement->as.var_decl.name, &statement->as.var_decl.type, initializer);
    }
    if (statement->kind == AST_ASSIGNMENT) {
        IrNode *value = lower_expression(statement->as.assignment.value);
        return ir_assign_create(statement->as.assignment.target->as.identifier.name, value);
    }
    if (statement->kind == AST_IF) {
        IrNode *condition = lower_expression(statement->as.if_stmt.condition);
        IrNode *then_branch = statement->as.if_stmt.then_branch == NULL ? NULL : lower_block(statement->as.if_stmt.then_branch);
        IrNode *else_branch = statement->as.if_stmt.else_branch == NULL ? NULL : lower_block(statement->as.if_stmt.else_branch);
        return ir_if_create(condition, then_branch, else_branch);
    }
    if (statement->kind == AST_WHILE) {
        IrNode *condition = lower_expression(statement->as.while_stmt.condition);
        IrNode *body = statement->as.while_stmt.body == NULL ? NULL : lower_block(statement->as.while_stmt.body);
        return ir_while_create(condition, body);
    }
    if (statement->kind == AST_BLOCK) {
        return lower_block(statement);
    }
    return NULL;
}

static IrNode *lower_block(const AstNode *block) {
    IrNode *result = ir_block_create(NULL);
    if (result == NULL) {
        return NULL;
    }
    for (size_t i = 0; i < block->as.block.statements.count; ++i) {
        IrNode *stmt = lower_statement(block->as.block.statements.items[i]);
        if (stmt == NULL) {
            ir_node_free(result);
            return NULL;
        }
        ir_list_append(&result->as.block.statements, stmt);
    }
    return result;
}

static IrNode *lower_function(const AstNode *function) {
    IrNode *ir_function = ir_function_create(function->as.function.name, &function->as.function.return_type);
    if (ir_function == NULL) {
        return NULL;
    }
    for (size_t i = 0; i < function->as.function.body.count; ++i) {
        IrNode *stmt = lower_statement(function->as.function.body.items[i]);
        if (stmt == NULL) {
            ir_node_free(ir_function);
            return NULL;
        }
        ir_list_append(&ir_function->as.function.body, stmt);
    }
    return ir_function;
}

static IrNode *lower_program(const Program *program) {
    if (program == NULL || program->functions.count == 0) {
        return NULL;
    }
    return lower_function(program->functions.items[0]);
}

int compile_source_file(const char *input_path, const char *output_path) {
    Source *source = source_from_file(input_path);
    if (source == NULL) {
        fprintf(stderr, "failed to read %s\n", input_path);
        return 1;
    }

    TokenList *tokens = lex_source(source);
    if (tokens == NULL) {
        fprintf(stderr, "failed to tokenize %s\n", input_path);
        source_free(source);
        return 1;
    }

    Program *program = parse_program(tokens);
    tokens_free(tokens);
    if (program == NULL) {
        fprintf(stderr, "failed to parse %s\n", input_path);
        source_free(source);
        return 1;
    }

    Diagnostics *diagnostics = nemantics_validate(program);
    if (diagnostics == NULL || diagnostics_has_errors(diagnostics)) {
        fprintf(stderr, "semantic validation failed for %s\n", input_path);
        diagnostics_free(diagnostics);
        program_free(program);
        source_free(source);
        return 1;
    }
    diagnostics_free(diagnostics);

    IrNode *ir = lower_program(program);
    program_free(program);
    source_free(source);
    if (ir == NULL) {
        fprintf(stderr, "failed to lower AST for %s\n", input_path);
        return 1;
    }

    int ok = codegen_emit_c(output_path, ir);
    ir_node_free(ir);
    if (!ok) {
        fprintf(stderr, "failed to emit %s\n", output_path);
        return 1;
    }

    return 0;
}
