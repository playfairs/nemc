#include "nem/source.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Source *source_from_file(const char *path) {
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        return NULL;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return NULL;
    }
    long length = ftell(file);
    if (length < 0) {
        fclose(file);
        return NULL;
    }
    if (fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }

    char *text = malloc((size_t)length + 1);
    if (text == NULL) {
        fclose(file);
        return NULL;
    }

    size_t read = fread(text, 1, (size_t)length, file);
    fclose(file);
    text[read] = '\0';

    Source *source = malloc(sizeof(Source));
    if (source == NULL) {
        free(text);
        return NULL;
    }
    source->path = strdup(path);
    source->text = text;
    source->length = (size_t)length;
    return source;
}

Source *source_from_text(const char *text, const char *path) {
    Source *source = malloc(sizeof(Source));
    if (source == NULL) {
        return NULL;
    }
    source->path = path == NULL ? NULL : strdup(path);
    source->text = strdup(text);
    source->length = strlen(text);
    return source;
}

void source_free(Source *source) {
    if (source == NULL) {
        return;
    }
    free(source->path);
    free(source->text);
    free(source);
}

void span_init(Span *span, size_t offset, size_t length, size_t line, size_t column) {
    span->offset = offset;
    span->length = length;
    span->line = line;
    span->column = column;
}
