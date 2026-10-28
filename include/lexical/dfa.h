#ifndef __LEXICAL_DFA_H__
#define __LEXICAL_DFA_H__

#include "automaton/state.h"

/// Builds the lexer's DFA into `reg` and returns the id of its root state.
///
/// Accepting states carry the `token_kind_t` they accept in `state_t.data`.
/// Inputs without a transition lead to `STATE_DEAD`.
state_id_t lexer_dfa_build(state_reg_t *reg);

#endif
