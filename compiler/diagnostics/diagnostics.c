#include "nem/diagnostics.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void diagnostics_init(Diagnostics *diagnostics) {
    diagnostics->items = NULL;
    diagnostics->count = 0;
    diagnostics->capacity = 0;
}

Diagnostics *diagnostics_create(void) {
    Diagnostics *diagnostics = calloc(1, sizeof(Diagnostics));
    if (diagnostics == NULL) {
        return NULL;
    }
    diagnostics_init(diagnostics);
    return diagnostics;
}

void diagnostics_free(Diagnostics *diagnostics) {
    if (diagnostics == NULL) {
        return;
    }
    for (size_t i = 0; i < diagnostics->count; ++i) {
        free(diagnostics->items[i].message);
    }
    free(diagnostics->items);
    free(diagnostics);
}

void diagnostics_add(Diagnostics *diagnostics, DiagnosticSeverity severity, const char *message, const Span *span) {
    if (diagnostics == NULL || message == NULL) {
        return;
    }
    if (diagnostics->count == diagnostics->capacity) {
        size_t new_capacity = diagnostics->capacity == 0 ? 8 : diagnostics->capacity * 2;
        Diagnostic *items = realloc(diagnostics->items, new_capacity * sizeof(Diagnostic));
        if (items == NULL) {
            return;
        }
        diagnostics->items = items;
        diagnostics->capacity = new_capacity;
    }

    Diagnostic *diagnostic = &diagnostics->items[diagnostics->count++];
    diagnostic->severity = severity;
    diagnostic->message = strdup(message);
    diagnostic->span = span == NULL ? (Span){0, 0, 0, 0} : *span;
}

int diagnostics_has_errors(const Diagnostics *diagnostics) {
    if (diagnostics == NULL) {
        return 0;
    }
    for (size_t i = 0; i < diagnostics->count; ++i) {
        if (diagnostics->items[i].severity == DIAGNOSTIC_ERROR) {
            return 1;
        }
    }
    return 0;
}

void diagnostics_print(const Diagnostics *diagnostics, const Source *source) {
    if (diagnostics == NULL) {
        return;
    }
    for (size_t i = 0; i < diagnostics->count; ++i) {
        const Diagnostic *diagnostic = &diagnostics->items[i];
        fprintf(stderr, "%s: %s\n", diagnostic->severity == DIAGNOSTIC_ERROR ? "error" : "warning", diagnostic->message);
        if (source != NULL && source->path != NULL && source->text != NULL) {
            fprintf(stderr, " --> %s:%zu:%zu\n", source->path, diagnostic->span.line, diagnostic->span.column);
        }
    }
}
