#ifndef __LEXICAL_ANALYZER_H__
#define __LEXICAL_ANALYZER_H__

#include "automaton/state.h"
#include "core/string/slice.h"

typedef struct
{
    state_reg_t state_reg;
    string_slice_t src;
    size_t offset, row, col;
} lexer_t;

void lexer_new(lexer_t *lexer, string_slice_t src);
void lexer_free(lexer_t *lexer);

void lexer_init_automaton(lexer_t *lexer);

void lexer_analyze(lexer_t *lexer);

#endif
