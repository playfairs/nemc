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

static IrNode *lower_function(const AstNode *function) {
    IrNode *ir_function = ir_function_create(function->as.function.name, &function->as.function.return_type);
    if (ir_function == NULL) {
        return NULL;
    }
    for (size_t i = 0; i < function->as.function.body.count; ++i) {
        AstNode *statement = function->as.function.body.items[i];
        if (statement->kind == AST_CALL) {
            Type call_type = statement->type;
            IrNode *call = ir_call_create(statement->as.call.name, &call_type);
            if (call == NULL) {
                ir_node_free(ir_function);
                return NULL;
            }
            for (size_t j = 0; j < statement->as.call.arguments.count; ++j) {
                AstNode *arg = statement->as.call.arguments.items[j];
                IrNode *arg_ir = NULL;
                if (arg->kind == AST_STRING_LITERAL) {
                    arg_ir = ir_string_create(arg->as.string_literal.text);
                } else if (arg->kind == AST_INTEGER_LITERAL) {
                    arg_ir = ir_constant_create(arg->as.integer_literal.value, &arg->type);
                } else {
                    arg_ir = ir_string_create("");
                }
                ir_list_append(&call->as.call.arguments, arg_ir);
            }
            ir_list_append(&ir_function->as.function.body, call);
        }
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
