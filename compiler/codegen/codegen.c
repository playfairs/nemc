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
