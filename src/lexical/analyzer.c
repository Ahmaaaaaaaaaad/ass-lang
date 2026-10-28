#include "lexical/analyzer.h"
#include "automaton/state.h"
#include "lexical/token.h"
#include "utils/guard.h"
#include <ctype.h>

void lexer_new(lexer_t *lexer, string_slice_t src)
{
    state_reg_init(&lexer->state_reg);
    lexer->src = src;
    lexer->offset = 0;
    lexer->row = lexer->col = 1;

    lexer_init_automaton(lexer);
}

void lexer_free(lexer_t *lexer) { state_reg_free(&lexer->state_reg); }

void lexer_init_automaton(lexer_t *lexer)
{
    state_reg_t *reg = &lexer->state_reg;
    state_id_t root_id = state_reg_register(reg, STF_ROOT, 0);

    // Keywords
    {
        for (size_t i = 0; i <= UINT8_MAX; ++i)
        {
            if (!isalpha(i) || i == '_')
                continue;

            state_reg_insert(reg, root_id, (uint8_t)i,
                             state_reg_register(reg, STF_NONE, 0));
        }

        for (size_t i = ____tok_kw_start + 1; i < ____tok_kw_end; ++i)
        {
            token_kind_t tk = (token_kind_t)i;
            string_slice_t repr = as_slice(token_kind_str_repr(tk));
            state_id_t curr_id = root_id;

            for (size_t j = 0; j < repr.n; ++j)
            {
                state_id_t c_state_id = state_reg_register(reg, STF_NONE, 0);
                state_t *curr = state_reg_get_mut(reg, curr_id);

                state_insert(curr, (uint8_t)repr.data[j], c_state_id);
                curr_id = state_step(curr, (uint8_t)repr.data[j]);
            }

            state_t *final = state_reg_get_mut(reg, curr_id);
            final->flag = STF_ACCEPTING;
            final->data = i;
        }
    }

    // String Literal Automaton
    {
        // \" (Head)
        state_id_t str_start_id = state_reg_register(reg, STF_NONE, 0);
        state_reg_insert(reg, root_id, '"', str_start_id);

        // \" (Tail)
        // state_reg_insert(reg, str_start_id, '"', str_end_id);

        // <char>*
        state_id_t str_char_id = state_reg_register(reg, STF_NONE, 0);

        state_t *str_char = state_reg_get_mut(reg, str_char_id);
        vec_update_cap(state_id_t)(&str_char->tr, UINT8_MAX + 1);

        state_id_t str_end_id = state_reg_register(reg, STF_ACCEPTING, TOK_STR);

        for (size_t i = 0; i <= UINT8_MAX; ++i)
        {
            bool is_qm = (char)i == '"', is_lf = (char)i == '\n';

            state_reg_insert(reg, str_start_id, (uint8_t)i,
                             is_qm   ? str_end_id
                             : is_lf ? SIZE_MAX
                                     : str_char_id);
            state_insert(str_char, (uint8_t)i,
                         is_qm   ? str_end_id
                         : is_lf ? SIZE_MAX
                                 : str_char_id);
        }
    }

    // Character Literal Automaton
    {
        // \' (Head)
        state_id_t c_start_id = state_reg_register(reg, STF_NONE, 0);

        // <char>
        state_id_t c_char_id = state_reg_register(reg, STF_NONE, 0);
        state_t *c_char = state_reg_get_mut(reg, c_char_id);

        // \' (Tail)
        state_id_t c_end_id = state_reg_register(reg, STF_ACCEPTING, TOK_CHAR);

        // TODO
        // // \[char] (Escaped)
        // // TEMP: escapes any character for the mean time
        // state_id_t escaped_id = state_reg_register(reg, STF_NONE, 0);

        // vec_update_cap(state_id_t)(&c_char->tr, UINT8_MAX + 1);
        for (size_t i = 0; i <= UINT8_MAX; ++i)
        {
            bool
                // is_bsl = (char)i == '\\',
                is_lf = (char)i == '\n';

            state_reg_insert(reg, c_start_id, (uint8_t)i,
                             // is_bsl  ? escaped_id :
                             is_lf ? SIZE_MAX : c_char_id);
            // state_reg_insert(reg, escaped_id, (uint8_t)i,
            //                  is_lf ? SIZE_MAX : c_char_id);
            state_insert(c_char, '\'', c_end_id);
        }

        state_reg_insert(reg, root_id, '\'', c_start_id);
    }

    // Numeric Literal Automaton (Int and Float)
    {
    }

    // Whitespaces
    {
        state_id_t ws_id = state_reg_register(reg, STF_ACCEPTING, 1);

        for (size_t i = 0; i <= UINT8_MAX; ++i)
        {
            if (!isspace(i))
                continue;

            state_reg_insert(reg, root_id, (char)i, ws_id);
        }
    }
}

bool lexer_eof(lexer_t *lexer) { return lexer->offset >= lexer->src.n; }

void lexer_analyze_token(lexer_t *lexer)
{
    (void)lexer;

    state_id_t curr_id = 0;
    for (size_t i = lexer->offset; i < lexer->src.n; ++i)
    {
        print("%zu -> ", curr_id);
        state_t const *curr = state_reg_get(&lexer->state_reg, curr_id);
        curr_id = state_step(curr, (uint8_t)lexer->src.data[i]);

        if (curr_id == SIZE_MAX)
        {
            println("[restart] %zu (%c)", curr_id, (char)lexer->src.data[i]);
            curr_id = 0;
            --i;
        }
        else
        {
            println("[accepted] %zu (%c)", curr_id, (char)lexer->src.data[i]);
        }

        // if (curr_id == SIZE_MAX)
        // {
        //     print("expects ");
        //     for (size_t i = 0; i <= UINT8_MAX; ++i)
        //     {
        //         state_id_t id = curr->tr.data[i];
        //         if (id == SIZE_MAX)
        //             continue;

        //         print("%c, ", (char)i);
        //     }

        //     println("");
        // }
    }

    // state_id_t curr_id = root_id;
    // for (size_t i = 0; i < seq.n; ++i)
    // {
    //     state_t const *curr = state_reg_get(reg, curr_id);
    //     curr_id = state_step(curr, (uint8_t)seq.data[i]);
    // }
}

void lexer_analyze(lexer_t *lexer)
{
    //
    lexer_analyze_token(lexer);
}
