#ifndef NEM_CODEGEN_H
#define NEM_CODEGEN_H

#include "nem/ir.h"

int codegen_emit_c(const char *path, const IrNode *program);

#endif
