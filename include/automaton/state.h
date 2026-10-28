#ifndef __AUTOMATON_STATE_H__
#define __AUTOMATON_STATE_H__

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "core/collection/hash_map.h"
#include "core/string/slice.h"
#include "core/utils/bi_defs.h"

#define is_init(state) ((state).flag != STF_UNINIT)
#define is_accepting(state) ((state).flag == STF_ACCEPTING)

typedef enum : uint8_t
{
    STF_UNINIT,
    STF_DEAD,
    STF_ROOT,
    STF_NONE,
    STF_ACCEPTING,
} state_flag_t;

typedef size_t state_id_t;
declare_vec(state_id_t);

/// The dead (trap) state. It is always registered first by `state_reg_init`
/// so that a zeroed transition slot naturally means "no transition".
#define STATE_DEAD ((state_id_t)0)

typedef struct state_t
{
    /// Transition table indexed by the input byte. Inputs at or past `tr.len`
    /// have no transition and lead to `STATE_DEAD`.
    vec(state_id_t) tr;
    state_flag_t flag;
    /// Payload of accepting states (e.g. the `token_kind_t` it accepts).
    uint8_t data;
} state_t;

declare_vec(state_t);
typedef struct
{
    vec(state_t) states;
} state_reg_t;

void state_reg_init(state_reg_t *reg);
void state_reg_free(state_reg_t *reg);

/// NOTE: Registering a state may reallocate the registry, invalidating every
/// `state_t *` previously returned by `state_reg_get(_mut)`. Prefer working
/// with `state_id_t`s and the `state_reg_*` functions.
state_id_t state_reg_register(state_reg_t *reg, state_flag_t flag,
                              uint8_t data);

state_t const *state_reg_get(state_reg_t *reg, state_id_t id);
state_t *state_reg_get_mut(state_reg_t *reg, state_id_t id);

void state_reg_insert(state_reg_t *reg, state_id_t state_id, uint8_t input,
                      state_id_t n_state_id);
void state_reg_insert_range(state_reg_t *reg, state_id_t state_id,
                            uint8_t from, uint8_t to, state_id_t n_state_id);
void state_reg_insert_set(state_reg_t *reg, state_id_t state_id,
                          char const *set, state_id_t n_state_id);

state_t const *state_step_from_id(state_reg_t *reg, state_id_t id,
                                  uint8_t input);

void state_init(state_t *state);
void state_free(state_t *state);

void state_insert(state_t *state, uint8_t input, state_id_t state_id);

state_id_t state_step(state_t const *state, uint8_t input);
state_id_t state_word_step(state_reg_t *reg, state_id_t root_id,
                           string_slice_t seq);

#endif
