#include "nem/driver.h"
#include "nem/lexer.h"
#include "nem/nemantics.h"
#include "nem/parser.h"
#include "nem/source.h"

#include <assert.h>
#include <stdio.h>

static void test_lexer() {
    const char *source_text = "fn main() { print(\"Hello, world!\"); }";
    Source *source = source_from_text(source_text, "example.nem");
    TokenList *tokens = lex_source(source);
    assert(tokens != NULL);
    assert(tokens->count > 0);
    source_free(source);
    tokens_free(tokens);
}

static void test_compile_minimal_program() {
    const char *input = "fn main() { print(\"Hello, world!\"); }";
    const char *input_path = "./build/test_input.nem";
    const char *output_path = "./build/test_output.c";

    FILE *input_file = fopen(input_path, "w");
    assert(input_file != NULL);
    fputs(input, input_file);
    fclose(input_file);

    Source *source = source_from_text(input, "example.nem");
    TokenList *tokens = lex_source(source);
    assert(tokens != NULL);
    Program *program = parse_program(tokens);
    assert(program != NULL);
    Diagnostics *diagnostics = nemantics_validate(program);
    assert(diagnostics != NULL);
    assert(!diagnostics_has_errors(diagnostics));
    diagnostics_free(diagnostics);
    tokens_free(tokens);
    program_free(program);
    source_free(source);

    assert(compile_source_file(input_path, output_path) == 0);
    FILE *output_file = fopen(output_path, "r");
    assert(output_file != NULL);
    fclose(output_file);
}

static void test_control_flow_program() {
    const char *input =
        "fn main() {"
        "   let mut x: int = 10;"
        "   while x < 20 {"
        "       if x == 15 {"
        "           print(\"halfway\");"
        "       }"
        "       x = x + 1;"
        "   }"
        "}";
    const char *input_path = "./build/test_control_flow.nem";
    const char *output_path = "./build/test_control_flow.c";

    FILE *input_file = fopen(input_path, "w");
    assert(input_file != NULL);
    fputs(input, input_file);
    fclose(input_file);

    assert(compile_source_file(input_path, output_path) == 0);
}

static void test_missing_return_rejected() {
    const char *input =
        "fn foo(x: int) -> int {"
        "   if x > 0 {"
        "       return 1;"
        "   }"
        "}";
    const char *input_path = "./build/test_missing_return.nem";
    const char *output_path = "./build/test_missing_return.c";

    FILE *input_file = fopen(input_path, "w");
    assert(input_file != NULL);
    fputs(input, input_file);
    fclose(input_file);

    assert(compile_source_file(input_path, output_path) != 0);
}

int main(void) {
    test_lexer();
    test_compile_minimal_program();
    test_control_flow_program();
    test_missing_return_rejected();
    return 0;
}
