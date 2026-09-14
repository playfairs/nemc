#ifndef NEM_PARSER_H
#define NEM_PARSER_H

#include "nem/ast.h"
#include "nem/lexer.h"

Program *parse_program(const TokenList *tokens);

#endif
