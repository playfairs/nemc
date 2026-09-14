#ifndef NEM_SOURCE_H
#define NEM_SOURCE_H

#include <stddef.h>

typedef struct {
    size_t line;
    size_t column;
    size_t offset;
    size_t length;
} Span;

typedef struct {
    char *path;
    char *text;
    size_t length;
} Source;

Source *source_from_file(const char *path);
Source *source_from_text(const char *text, const char *path);
void source_free(Source *source);
void span_init(Span *span, size_t offset, size_t length, size_t line, size_t column);

#endif
