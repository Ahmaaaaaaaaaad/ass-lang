#ifndef __LEXICAL_ANALYZER_H__
#define __LEXICAL_ANALYZER_H__

#include <stddef.h>

#include "automaton/state.h"
#include "core/string/slice.h"
#include "lexical/token.h"

typedef struct
{
    char const *file_path;
    string_slice_t src;

    state_reg_t dfa;
    state_id_t root_id;

    vec(token_t) tokens;
    size_t err_count;
} lexer_t;

void lexer_new(lexer_t *lexer, char const *file_path, string_slice_t src);
void lexer_free(lexer_t *lexer);

/// Tokenizes the whole source into `lexer->tokens`, skipping whitespace.
/// Lexical errors are reported to stderr, counted in `lexer->err_count` and
/// emitted as `TOK_UNKNOWN` tokens so analysis can continue past them.
void lexer_analyze(lexer_t *lexer);

void lexer_print_tokens(lexer_t const *lexer);

#endif
