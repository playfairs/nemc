#ifndef NEM_DIAGNOSTICS_H
#define NEM_DIAGNOSTICS_H

#include <stddef.h>

#include "nem/source.h"

typedef enum {
    DIAGNOSTIC_ERROR,
    DIAGNOSTIC_WARNING
} DiagnosticSeverity;

typedef struct {
    DiagnosticSeverity severity;
    char *message;
    Span span;
} Diagnostic;

typedef struct {
    Diagnostic *items;
    size_t count;
    size_t capacity;
} Diagnostics;

Diagnostics *diagnostics_create(void);
void diagnostics_free(Diagnostics *diagnostics);
void diagnostics_add(Diagnostics *diagnostics, DiagnosticSeverity severity, const char *message, const Span *span);
int diagnostics_has_errors(const Diagnostics *diagnostics);
void diagnostics_print(const Diagnostics *diagnostics, const Source *source);

#endif
