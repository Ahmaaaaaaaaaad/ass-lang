#include <stdint.h>

#include "automaton/state.h"
#include "lexical/dfa.h"
#include "lexical/token.h"

/// Characters allowed after a backslash in string and char literals.
#define ESCAPE_CHARS "ntr0\\'\""
#define WHITESPACE_CHARS " \t\r\n"

static void insert_ident_start_chars(state_reg_t *reg, state_id_t id,
                                     state_id_t dest_id)
{
    state_reg_insert_range(reg, id, 'a', 'z', dest_id);
    state_reg_insert_range(reg, id, 'A', 'Z', dest_id);
    state_reg_insert(reg, id, '_', dest_id);
}

static void insert_ident_chars(state_reg_t *reg, state_id_t id,
                               state_id_t dest_id)
{
    insert_ident_start_chars(reg, id, dest_id);
    state_reg_insert_range(reg, id, '0', '9', dest_id);
}

// <ws>+
static void init_whitespace_automaton(state_reg_t *reg, state_id_t root_id)
{
    state_id_t ws_id = state_reg_register(reg, STF_ACCEPTING, TOK_WHITESPACE);

    state_reg_insert_set(reg, root_id, WHITESPACE_CHARS, ws_id);
    state_reg_insert_set(reg, ws_id, WHITESPACE_CHARS, ws_id);
}

// Identifiers: [A-Za-z_][A-Za-z0-9_]*
//
// Keywords are paths branching off the identifier automaton. Every state along
// a keyword's path accepts `TOK_IDENT` (`wh` is a valid identifier) and leaves
// to the generic identifier state on any other identifier character, while the
// last state of the path accepts the keyword itself:
//
//   () -i-> ((i)) -f-> ((if: KW_IF))
//              |             |
//              | [^f]        | [A-Za-z0-9_]
//              v             v
//         ((ident: IDENT)) <-+
//              ^   |
//              +---+ [A-Za-z0-9_]
static void init_ident_kw_automaton(state_reg_t *reg, state_id_t root_id)
{
    state_id_t ident_id = state_reg_register(reg, STF_ACCEPTING, TOK_IDENT);

    insert_ident_start_chars(reg, root_id, ident_id);
    insert_ident_chars(reg, ident_id, ident_id);

    for (size_t kind = ____tok_kw_start + 1; kind < ____tok_kw_end; ++kind)
    {
        char const *kw = token_kind_str_repr((token_kind_t)kind);

        state_id_t curr_id = root_id;
        for (char const *c = kw; *c != '\0'; ++c)
        {
            state_id_t next_id =
                state_step(state_reg_get(reg, curr_id), (uint8_t)*c);

            // Branch off the generic identifier state unless another keyword
            // already created this prefix.
            if (next_id == ident_id || next_id == STATE_DEAD)
            {
                next_id = state_reg_register(reg, STF_ACCEPTING, TOK_IDENT);
                insert_ident_chars(reg, next_id, ident_id);
                state_reg_insert(reg, curr_id, (uint8_t)*c, next_id);
            }

            curr_id = next_id;
        }

        state_reg_get_mut(reg, curr_id)->data = (uint8_t)kind;
    }
}

// Integers: <digit>+
// Floats:   <digit>+ \. <digit>+
static void init_num_lit_automaton(state_reg_t *reg, state_id_t root_id)
{
    // <digit>+
    state_id_t digit_id = state_reg_register(reg, STF_ACCEPTING, TOK_INT);
    state_reg_insert_range(reg, root_id, '0', '9', digit_id);
    state_reg_insert_range(reg, digit_id, '0', '9', digit_id);

    // \.
    state_id_t frac_sep_id = state_reg_register(reg, STF_NONE, 0);
    state_reg_insert(reg, digit_id, '.', frac_sep_id);

    // <frac_digit>+
    state_id_t frac_digit_id =
        state_reg_register(reg, STF_ACCEPTING, TOK_FLOAT);
    state_reg_insert_range(reg, frac_sep_id, '0', '9', frac_digit_id);
    state_reg_insert_range(reg, frac_digit_id, '0', '9', frac_digit_id);
}

// Strings: \" (<char> | \\ <escape>)* \"
// where <char> is any byte except \", \\ and a newline.
static void init_str_lit_automaton(state_reg_t *reg, state_id_t root_id)
{
    state_id_t body_id = state_reg_register(reg, STF_NONE, 0);
    state_id_t esc_id = state_reg_register(reg, STF_NONE, 0);
    state_id_t end_id = state_reg_register(reg, STF_ACCEPTING, TOK_STR);

    state_reg_insert(reg, root_id, '"', body_id);

    state_reg_insert_range(reg, body_id, 0, UINT8_MAX, body_id);
    state_reg_insert(reg, body_id, '\n', STATE_DEAD);
    state_reg_insert(reg, body_id, '\\', esc_id);
    state_reg_insert(reg, body_id, '"', end_id);

    state_reg_insert_set(reg, esc_id, ESCAPE_CHARS, body_id);
}

// Chars: \' (<char> | \\ <escape>) \'
// where <char> is any byte except \', \\ and a newline.
static void init_char_lit_automaton(state_reg_t *reg, state_id_t root_id)
{
    state_id_t open_id = state_reg_register(reg, STF_NONE, 0);
    state_id_t esc_id = state_reg_register(reg, STF_NONE, 0);
    state_id_t char_id = state_reg_register(reg, STF_NONE, 0);
    state_id_t end_id = state_reg_register(reg, STF_ACCEPTING, TOK_CHAR);

    state_reg_insert(reg, root_id, '\'', open_id);

    state_reg_insert_range(reg, open_id, 0, UINT8_MAX, char_id);
    state_reg_insert(reg, open_id, '\n', STATE_DEAD);
    state_reg_insert(reg, open_id, '\'', STATE_DEAD);
    state_reg_insert(reg, open_id, '\\', esc_id);

    state_reg_insert_set(reg, esc_id, ESCAPE_CHARS, char_id);
    state_reg_insert(reg, char_id, '\'', end_id);
}

// Operators and delimiters, all single characters.
static void init_symbol_automaton(state_reg_t *reg, state_id_t root_id)
{
    token_kind_t const kinds[] = {
        TOK_PLUS,   TOK_MINUS,      TOK_STAR,   TOK_SLASH,
        TOK_PERCENT, TOK_DOT,       TOK_COMMA,  TOK_SEMI_COLON,
        TOK_LBRACE, TOK_RBRACE,     TOK_LPAREN, TOK_RPAREN,
        TOK_LBRACKET, TOK_RBRACKET,
    };

    for (size_t i = 0; i < (sizeof(kinds) / sizeof(kinds[0])); ++i)
    {
        char const *repr = token_kind_str_repr(kinds[i]);
        state_id_t sym_id = state_reg_register(reg, STF_ACCEPTING, kinds[i]);
        state_reg_insert(reg, root_id, (uint8_t)repr[0], sym_id);
    }
}

state_id_t lexer_dfa_build(state_reg_t *reg)
{
    state_id_t root_id = state_reg_register(reg, STF_ROOT, 0);

    init_whitespace_automaton(reg, root_id);
    init_ident_kw_automaton(reg, root_id);
    init_num_lit_automaton(reg, root_id);
    init_str_lit_automaton(reg, root_id);
    init_char_lit_automaton(reg, root_id);
    init_symbol_automaton(reg, root_id);

    return root_id;
}
