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

extern size_t const LOWER_ALPHA_OFS, UPPER_ALPHA_OFS, NUM_OFS, SYM_OFS;
extern size_t const TR_TABLE_SIZE;

typedef enum : uint8_t
{
    STRI_EXACT,
    STRI_SELECTION,
    STRI_RANGED,
} state_tr_input_kind_t;

typedef enum : uint8_t
{
    STF_UNINIT,
    STF_ROOT,
    STF_NONE,
    STF_ACCEPTING,
} state_flag_t;

typedef size_t state_id_t;
declare_vec(state_id_t);

// TODO?: declare_hm(uint8_t, state_id_t);

typedef struct state_t
{
    // hash_map(uint8_t, state_id_t) tr;
    vec(state_id_t) tr;
    state_id_t fallback;
    state_flag_t flag;
    uint8_t data;
} state_t;

declare_vec(state_t);
typedef struct
{
    vec(state_t) states;
} state_reg_t;

void state_reg_init(state_reg_t *reg);
void state_reg_free(state_reg_t *reg);

state_id_t state_reg_register(state_reg_t *reg, state_flag_t flag,
                              uint8_t data);
state_t const *state_reg_register_and_get(state_reg_t *reg, state_flag_t flag,
                                          uint8_t data);
state_t *state_reg_register_and_get_mut(state_reg_t *reg, state_flag_t flag,
                                        uint8_t data);

state_t const *state_reg_get(state_reg_t *reg, state_id_t id);
state_t *state_reg_get_mut(state_reg_t *reg, state_id_t id);

void state_reg_insert(state_reg_t *reg, state_id_t state_id, uint8_t input,
                      state_id_t n_state_id);

state_t const *state_step_from_id(state_reg_t *reg, state_id_t id,
                                  uint8_t input);

size_t char_idx_of(char c);

void state_init(state_t *state);
void state_free(state_t *state);

void state_insert(state_t *state, uint8_t input, state_id_t state_id);

state_id_t state_step(state_t const *state, uint8_t input);
state_id_t state_word_step(state_reg_t *reg, state_id_t root_id,
                           string_slice_t seq);

// void state_multi_insert(state_t *state, slice(char) char_seq, uint8_t
// acc_data);

// uint8_t state_get(state_t *root, slice(char) char_seq);

#endif
