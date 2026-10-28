#include <stdlib.h>

#include "automaton/state.h"
#include "core/string/slice.h"
#include "lexical/token.h"
#include "utils/guard.h"

define_vec(state_id_t);
define_vec(state_t);

// #define PACKED

size_t const LOWER_ALPHA_OFS = 0, UPPER_ALPHA_OFS = ('z' - 'a') + 1,
             NUM_OFS = UPPER_ALPHA_OFS + (('Z' - 'A') + 1),
             SYM_OFS = NUM_OFS + (('9' - '0') + 1);

size_t const TR_TABLE_SIZE =
#ifdef PACKED
    SYM_OFS + 1 /* NOTE: Number of symbols
                 */
#else
    UINT8_MAX + 1
#endif
    ;

#ifdef PACKED
size_t char_idx_of(char c)
{
    if (isalnum(c))
    {
        return isalpha(c) ? islower(c) ? LOWER_ALPHA_OFS + (c - 'a')
                                       : UPPER_ALPHA_OFS + (c - 'A')
                          : NUM_OFS + (c - '0');
    }

    if (c != '_')
        panic("invalid character");

    return SYM_OFS;
}
#else
size_t char_idx_of(char c) { return (size_t)c; }
#endif
#undef PACKED

void state_reg_init(state_reg_t *reg)
{
    vec_new(state_t)(&reg->states, state_free, 4);
}

void state_reg_free(state_reg_t *reg) { vec_free(state_t)(&reg->states); }

state_id_t state_reg_register(state_reg_t *reg, state_flag_t flag, uint8_t data)
{
    state_id_t id = reg->states.len;
    state_t *state = vec_expand(state_t)(&reg->states);

    state_init(state);
    state->flag = flag, state->data = data;

    return id;
}

state_t const *state_reg_register_and_get(state_reg_t *reg, state_flag_t flag,
                                          uint8_t data)
{
    state_id_t id = state_reg_register(reg, flag, data);
    return state_reg_get(reg, id);
}

state_t *state_reg_register_and_get_mut(state_reg_t *reg, state_flag_t flag,
                                        uint8_t data)
{
    state_id_t id = state_reg_register(reg, flag, data);
    return state_reg_get_mut(reg, id);
}

state_t const *state_reg_get(state_reg_t *reg, state_id_t id)
{
    return vec_get(state_t)(&reg->states, id);
}

state_t *state_reg_get_mut(state_reg_t *reg, state_id_t id)
{
    return vec_get_mut(state_t)(&reg->states, id);
}

void state_reg_insert(state_reg_t *reg, state_id_t state_id, uint8_t input,
                      state_id_t n_state_id)
{
    state_t *state = state_reg_get_mut(reg, state_id);
    state_insert(state, input, n_state_id);
}

state_t const *state_step_from_id(state_reg_t *reg, state_id_t id,
                                  uint8_t input)
{
    state_t const *state = state_reg_get(reg, id);
    state_id_t next_id = state_step(state, input);

    return state_reg_get(reg, next_id);
}

void state_init(state_t *state)
{
    *state = (state_t){0};
    vec_new(state_id_t)(&state->tr, NULL, 4);
}

void state_free(state_t *state) { vec_free(state_id_t)(&state->tr); }

void state_insert(state_t *state, uint8_t input, state_id_t state_id)
{
    size_t initial = state->tr.cap;
    vec_update_cap(state_id_t)(&state->tr, (size_t)input + 1);

    // Capacity increased
    if (initial < state->tr.cap)
    {
        for (size_t i = initial; i < state->tr.cap; i++)
            state->tr.data[i] = SIZE_MAX;
    }

    state->tr.data[(size_t)input] = state_id;
}

state_id_t state_step(state_t const *state, uint8_t input)
{
    return input >= state->tr.cap ? SIZE_MAX : state->tr.data[(size_t)input];
}

state_id_t state_word_step(state_reg_t *reg, state_id_t root_id,
                           string_slice_t seq)
{
    state_id_t curr_id = root_id;
    for (size_t i = 0; i < seq.n; ++i)
    {
        state_t const *curr = state_reg_get(reg, curr_id);
        curr_id = state_step(curr, (uint8_t)seq.data[i]);
    }

    return curr_id;
}

// void state_multi_insert(state_t *state, slice(char) char_seq, uint8_t
// acc_data)
// {
//     if (!is_init(*state))
//         state_init(state);
//     state_t *curr = state;

//     for (size_t i = 0; i < char_seq.n; ++i)
//     {
//         char c = char_seq.data[i];
//         state_t *c_state = curr->tr_table + char_idx_of(c);

//         if (is_occupied(*c_state))
//             state_multi_insert(
//                 c_state, as_slicen(char_seq.data + i + 1, char_seq.n - (i
//                 + 1)), acc_data);
//         else
//         {
//             state_init(c_state);
//             c_state->input = c;
//         }

//         curr = c_state;
//     }

//     curr->data = acc_data;
// }

// state_t *state_insert(state_t *state, char input_sym, uint8_t data)
// {
//     state_multi_insert(state, as_slicen((char const[]){input_sym}, 1),
//     data); return state->tr_table + char_idx_of(input_sym);
// }

// uint8_t state_get(state_t *root, slice(char) char_seq)
// {
//     assert(is_init((*root)), "cannot retrieve data of uninitialized
//     states");

//     state_t *curr = root;
//     for (size_t i = 0; i < char_seq.n; ++i)
//     {
//         char c = char_seq.data[i];
//         state_t *c_state = curr->tr_table + char_idx_of(c);

//         if (!is_occupied((*c_state)))
//             return c_state->data;

//         curr = c_state;
//     }

//     return curr->data;
// }
