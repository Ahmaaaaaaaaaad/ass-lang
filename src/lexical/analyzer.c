#include <stdint.h>
#include <stdio.h>

#include "automaton/state.h"
#include "lexical/analyzer.h"
#include "lexical/dfa.h"
#include "lexical/token.h"
#include "utils/log.h"

void lexer_new(lexer_t *lexer, char const *file_path, string_slice_t src)
{
    lexer->file_path = file_path;
    lexer->src = src;
    lexer->err_count = 0;

    state_reg_init(&lexer->dfa);
    lexer->root_id = lexer_dfa_build(&lexer->dfa);

    vec_new(token_t)(&lexer->tokens, NULL, 64);
}

void lexer_free(lexer_t *lexer)
{
    vec_free(token_t)(&lexer->tokens);
    state_reg_free(&lexer->dfa);
}

/// Maximal munch: runs the DFA from `pos` until it reaches the dead state and
/// returns the length of the longest accepted prefix (0 if none), storing the
/// kind of that prefix in `kind`.
static size_t lexer_munch(lexer_t *lexer, size_t pos, token_kind_t *kind)
{
    size_t acc_len = 0;
    state_id_t curr_id = lexer->root_id;

    for (size_t i = pos; i < lexer->src.n; ++i)
    {
        state_t const *curr = state_reg_get(&lexer->dfa, curr_id);
        curr_id = state_step(curr, (uint8_t)lexer->src.data[i]);
        if (curr_id == STATE_DEAD)
            break;

        state_t const *next = state_reg_get(&lexer->dfa, curr_id);
        if (is_accepting(*next))
        {
            acc_len = (i - pos) + 1;
            *kind = (token_kind_t)next->data;
        }
    }

    return acc_len;
}

/// Picks how much input to skip after a lexical error at `pos`, so that a bad
/// literal is reported once instead of cascading into more errors.
static size_t lexer_recover(lexer_t *lexer, size_t pos, char const **reason)
{
    char quote = lexer->src.data[pos];
    if (quote != '"' && quote != '\'')
    {
        *reason = "unrecognized character";
        return 1;
    }

    *reason = quote == '"' ? "unterminated or malformed string literal"
                           : "unterminated or malformed character literal";

    // Skip to the closing quote on the same line, or to the end of the line.
    size_t i = pos + 1;
    for (; i < lexer->src.n && lexer->src.data[i] != '\n'; ++i)
    {
        char c = lexer->src.data[i];
        if (c == '\\' && i + 1 < lexer->src.n && lexer->src.data[i + 1] != '\n')
            ++i;
        else if (c == quote)
            return (i - pos) + 1;
    }

    // Leave the `\r` of a `\r\n` line ending to the whitespace automaton.
    if (i > pos + 1 && lexer->src.data[i - 1] == '\r')
        --i;

    return i - pos;
}

void lexer_analyze(lexer_t *lexer)
{
    size_t pos = 0;
    uint32_t line = 1, col = 1;

    while (pos < lexer->src.n)
    {
        token_t tok = {.line = line, .col = col};
        size_t len = lexer_munch(lexer, pos, &tok.kind);

        if (len == 0)
        {
            char const *reason;
            len = lexer_recover(lexer, pos, &reason);
            tok.kind = TOK_UNKNOWN;

            eprintln("%s:%u:%u: error: %s: `%.*s`", lexer->file_path, line, col,
                     reason, (int)len, lexer->src.data + pos);
            ++lexer->err_count;
        }

        tok.lexeme = as_slicen(lexer->src.data + pos, len);
        if (tok.kind != TOK_WHITESPACE)
            vec_push(token_t)(&lexer->tokens, tok);

        for (size_t i = pos; i < pos + len; ++i)
        {
            if (lexer->src.data[i] == '\n')
                ++line, col = 1;
            else
                ++col;
        }

        pos += len;
    }
}

void lexer_print_tokens(lexer_t const *lexer)
{
    println("%-10s %-12s %s", "LINE:COL", "KIND", "LEXEME");
    vec_for_each(token_t, tok, lexer->tokens, {
        char pos[24];
        snprintf(pos, sizeof(pos), "%u:%u", tok->line, tok->col);
        println("%-10s %-12s " slice_fmt, pos, token_kind_name(tok->kind),
                use(tok->lexeme));
    });
}
