#include <stdlib.h>

#include "automaton/state.h"
#include "core/string/slice.h"
#include "utils/control.h"
#include "utils/guard.h"
#include "utils/log.h"

define_vec(state_id_t);
define_vec(state_t);

void state_reg_init(state_reg_t *reg)
{
    vec_new(state_t)(&reg->states, state_free, 4);

    state_id_t dead_id = state_reg_register(reg, STF_DEAD, 0);
    assert(dead_id == STATE_DEAD, "dead state must be the first state");
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

state_t const *state_reg_get(state_reg_t *reg, state_id_t id)
{
    return vec_get_checked(state_t)(&reg->states, id);
}

state_t *state_reg_get_mut(state_reg_t *reg, state_id_t id)
{
    return vec_get_mut_checked(state_t)(&reg->states, id);
}

void state_reg_insert(state_reg_t *reg, state_id_t state_id, uint8_t input,
                      state_id_t n_state_id)
{
    state_t *state = state_reg_get_mut(reg, state_id);
    state_insert(state, input, n_state_id);
}

void state_reg_insert_range(state_reg_t *reg, state_id_t state_id,
                            uint8_t from, uint8_t to, state_id_t n_state_id)
{
    state_t *state = state_reg_get_mut(reg, state_id);
    for (size_t i = from; i <= to; ++i)
        state_insert(state, (uint8_t)i, n_state_id);
}

void state_reg_insert_set(state_reg_t *reg, state_id_t state_id,
                          char const *set, state_id_t n_state_id)
{
    state_t *state = state_reg_get_mut(reg, state_id);
    for (char const *c = set; *c != '\0'; ++c)
        state_insert(state, (uint8_t)*c, n_state_id);
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
    size_t idx = (size_t)input;

    // Grow the table up to `input`, filling the gap with dead transitions.
    vec_update_cap(state_id_t)(&state->tr, idx + 1);
    while (state->tr.len <= idx)
        state->tr.data[state->tr.len++] = STATE_DEAD;

    state->tr.data[idx] = state_id;
}

state_id_t state_step(state_t const *state, uint8_t input)
{
    size_t idx = (size_t)input;
    return idx < state->tr.len ? state->tr.data[idx] : STATE_DEAD;
}

state_id_t state_word_step(state_reg_t *reg, state_id_t root_id,
                           string_slice_t seq)
{
    state_id_t curr_id = root_id;
    for (size_t i = 0; i < seq.n && curr_id != STATE_DEAD; ++i)
    {
        state_t const *curr = state_reg_get(reg, curr_id);
        curr_id = state_step(curr, (uint8_t)seq.data[i]);
    }

    return curr_id;
}
