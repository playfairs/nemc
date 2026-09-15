#include "nem/codegen.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void emit_string(FILE *out, const char *text) {
    fputc('"', out);
    for (const unsigned char *cursor = (const unsigned char *)text; *cursor != '\0'; ++cursor) {
        switch (*cursor) {
            case '\\': fputs("\\\\", out); break;
            case '"': fputs("\\\"", out); break;
            case '\n': fputs("\\n", out); break;
            case '\r': fputs("\\r", out); break;
            case '\t': fputs("\\t", out); break;
            default: fputc(*cursor, out); break;
        }
    }
    fputc('"', out);
}

static void emit_ir(FILE *out, const IrNode *node) {
    if (node == NULL) {
        return;
    }
    if (node->kind == IR_FUNCTION) {
        fprintf(out, "int %s(void) {\n", node->as.function.name);
        for (size_t i = 0; i < node->as.function.body.count; ++i) {
            emit_ir(out, node->as.function.body.items[i]);
        }
        if (strcmp(node->as.function.name, "main") == 0) {
            fprintf(out, "    return 0;\n");
        }
        fprintf(out, "}\n");
    } else if (node->kind == IR_CALL) {
        if (strcmp(node->as.call.name, "print") == 0) {
            fprintf(out, "    puts(");
            if (node->as.call.arguments.count > 0) {
                emit_ir(out, node->as.call.arguments.items[0]);
            }
            fprintf(out, ");\n");
        } else {
            fprintf(out, "    %s(", node->as.call.name);
            for (size_t i = 0; i < node->as.call.arguments.count; ++i) {
                emit_ir(out, node->as.call.arguments.items[i]);
                if (i + 1 < node->as.call.arguments.count) {
                    fprintf(out, ", ");
                }
            }
            fprintf(out, ");\n");
        }
    } else if (node->kind == IR_STRING) {
        emit_string(out, node->as.string_literal.text);
    } else if (node->kind == IR_BOOL) {
        fprintf(out, "%s", node->as.bool_literal.value ? "1" : "0");
    } else if (node->kind == IR_CONSTANT) {
        fprintf(out, "%lld", node->as.constant.value);
    } else if (node->kind == IR_BINARY) {
        fprintf(out, "(");
        emit_ir(out, node->as.binary.left);
        fprintf(out, " %c ", node->as.binary.op);
        emit_ir(out, node->as.binary.right);
        fprintf(out, ")");
    } else if (node->kind == IR_UNARY) {
        fprintf(out, "%c(", node->as.unary.op);
        emit_ir(out, node->as.unary.value);
        fprintf(out, ")");
    } else if (node->kind == IR_DECL) {
        fprintf(out, "    %s %s = ", node->as.decl.type.kind == TYPE_KIND_STRING ? "char *" : "int", node->as.decl.name);
        if (node->as.decl.initializer != NULL) {
            emit_ir(out, node->as.decl.initializer);
        }
        fprintf(out, ";\n");
    } else if (node->kind == IR_ASSIGN) {
        fprintf(out, "    %s = ", node->as.assign.name);
        if (node->as.assign.value != NULL) {
            emit_ir(out, node->as.assign.value);
        }
        fprintf(out, ";\n");
    } else if (node->kind == IR_RETURN) {
        fprintf(out, "    return ");
        if (node->as.return_stmt.value != NULL) {
            emit_ir(out, node->as.return_stmt.value);
        }
        fprintf(out, ";\n");
    } else if (node->kind == IR_IF) {
        fprintf(out, "    if (");
        emit_ir(out, node->as.if_stmt.condition);
        fprintf(out, ") {\n");
        if (node->as.if_stmt.then_branch != NULL) {
            emit_ir(out, node->as.if_stmt.then_branch);
        }
        fprintf(out, "    }");
        if (node->as.if_stmt.else_branch != NULL) {
            fprintf(out, " else {\n");
            emit_ir(out, node->as.if_stmt.else_branch);
            fprintf(out, "    }\n");
        } else {
            fprintf(out, "\n");
        }
    } else if (node->kind == IR_WHILE) {
        fprintf(out, "    while (");
        emit_ir(out, node->as.while_stmt.condition);
        fprintf(out, ") {\n");
        if (node->as.while_stmt.body != NULL) {
            emit_ir(out, node->as.while_stmt.body);
        }
        fprintf(out, "    }\n");
    } else if (node->kind == IR_BLOCK) {
        for (size_t i = 0; i < node->as.block.statements.count; ++i) {
            emit_ir(out, node->as.block.statements.items[i]);
        }
    }
}

int codegen_emit_c(const char *path, const IrNode *program) {
    FILE *out = fopen(path, "w");
    if (out == NULL) {
        return 0;
    }
    fprintf(out, "#include <stdio.h>\n\n");
    if (program != NULL && program->kind == IR_FUNCTION) {
        emit_ir(out, program);
    }
    fclose(out);
    return 1;
}
