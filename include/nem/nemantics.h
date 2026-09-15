#ifndef NEM_NEMANTICS_H
#define NEM_NEMANTICS_H

#include "nem/ast.h"
#include "nem/diagnostics.h"

typedef struct Scope Scope;

typedef struct {
    char *name;
    Type type;
    int is_parameter;
    int is_function;
    int is_defined;
    int is_mutable;
} Symbol;

typedef struct {
    Symbol *items;
    size_t count;
    size_t capacity;
} SymbolTable;

struct Scope {
    char *name;
    SymbolTable symbols;
    Scope *parent;
};

Diagnostics *nemantics_validate(const Program *program);
void scope_push(Scope *scope, Scope **current);
void scope_pop(Scope **current);
void scope_define_symbol(Scope *scope, const char *name, const Type *type, int is_parameter, int is_function, int is_mutable);
Symbol *scope_lookup(Scope *scope, const char *name);

#endif
