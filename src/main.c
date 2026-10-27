#include "automaton/state.h"
#include "core/collection/hash_map.h"
#include "core/collection/slice.h"
#include "program/program.h"
#include "utils/drop.h"
#include "utils/guard.h"

#include <stdint.h>
#include <string.h>

// void insert_string_state_logic(state_t *root)
// {
//     //
//     state_t *new = state_insert(root, '"', 0);
//     for (size_t i = 1; i <= (size_t)(UINT8_MAX) + 1; ++i)
//     {
//         // println("new->tr_table: %p, c: %c", new->tr_table, (char)i);
//         state_t *curr = state_insert(new, (char)i, 0);
//         memcpy(curr->tr_table, new->tr_table, sizeof(state_t) *
//         TR_TABLE_SIZE);

//         if (curr->input == '"')
//             curr->data = 69;
//     }
// }

void init_str_lit_automaton(state_reg_t *reg, state_id_t root_id)
{
    // \" (Head)
    state_id_t str_start_id = state_reg_register(reg, STF_NONE, 0);

    // \" (Tail)
    state_id_t str_end_id = state_reg_register(reg, STF_ACCEPTING, 67);
    state_reg_insert(reg, str_start_id, '"', str_end_id);

    // <char>+
    state_id_t str_char_id = state_reg_register(reg, STF_NONE, 0);
    state_t *str_char = state_reg_get_mut(reg, str_char_id);

    vec_update_cap(state_id_t)(&str_char->tr, (size_t)UINT8_MAX + 1);
    for (size_t i = 0; i <= UINT8_MAX; ++i)
    {
        if ((char)i != '\n')
            state_insert(str_char, (uint8_t)i,
                         (char)i == '"' ? str_end_id : str_char_id);
    }

    state_reg_insert(reg, root_id, '"', str_start_id);
}

void init_num_lit_automaton(state_reg_t *reg, state_id_t root_id)
{
    state_id_t digit_id = state_reg_register(reg, STF_ACCEPTING, 68);
    state_t *digit = state_reg_get_mut(reg, digit_id);

    vec_update_cap(state_id_t)(&digit->tr, (size_t)'9');
    for (size_t i = '0'; i <= '9'; ++i)
    {
        state_reg_insert(reg, root_id, (uint8_t)i, digit_id);
        state_insert(digit, (uint8_t)i, digit_id);
    }
}

int main()
{
    // () -> \" -> <char>*
    //        \        |
    //         \------>\"

    drop(state_reg_free) state_reg_t reg;
    state_reg_init(&reg);

    state_id_t root_id = state_reg_register(&reg, STF_ROOT, 0);

    init_str_lit_automaton(&reg, root_id);
    init_num_lit_automaton(&reg, root_id);

    char const *samples[] = {"\"t", "\"test\"", "\"test\" ", "\"test\n\"",
                             "1",   "12",       "012", "01b"};
    for (size_t i = 0; i < (sizeof(samples) / sizeof(samples[0])); ++i)
    {
        state_id_t final_id =
            state_word_step(&reg, root_id, as_slice(samples[i]));
        state_t const *final = state_reg_get(&reg, final_id);
        println("sample: %s, final_id: %zu, final->flag: %u, final->data: %u",
                samples[i], final_id, final->flag, final->data);
    }

    return 0;
}

// int main() {
// // if (argc < 2)
// //     todo("display help information");

// drop(state_free) state_t root;
// state_init(&root);

// for (size_t i = ____tok_kw_start + 1; i < ____tok_kw_end; ++i)
// {
//     state_multi_insert(&root,
//                        as_slice(token_kind_str_repr((token_kind_t)i)),
//                        i);
// }

// state_print(&root, 0);

// uint8_t data = state_get(&root, as_slice("el"));
// if (data != 0)
// {
//     token_kind_t kind = data;
//     println("kind: %s", token_kind_str_repr(kind));
// }

// string_slice_t command = as_slice(argv[1]);

// drop(vec_free(string_slice_t)) vec(string_slice_t) arguments;
// vec_new(string_slice_t)(&arguments, NULL, (size_t)argc);
// for (size_t i = 2; i < (size_t)argc; ++i)
//     *vec_expand(string_slice_t)(&arguments) =
//         (string_slice_t){.data = argv[i], .n = strlen(argv[i])};

// drop(cmd_manager_free) cmd_manager_t cmdm;
// cmd_manager_new(&cmdm);

// init_cmds(&cmdm);

// cmd_t const *cmd = hm_get(string_slice_t, cmd_t)(&cmdm.cmds, &command);
// if (cmd == NULL)
//     todo("throw error: unknown command '" slice_fmt "'.", use(command));

// cmd->handler(cmd, view(string_slice_t)(&arguments, 0, arguments.len));

// }

typedef struct
{
    string_slice_t name, sh;
} cmd_opt_t;

typedef struct cmd_t
{
    string_slice_t name;
    void (*handler)(struct cmd_t const *cmd, slice(string_slice_t) arguments);
} cmd_t;

declare_hm(string_slice_t, cmd_t);
define_hm(string_slice_t, cmd_t);

typedef struct
{
    hash_map(string_slice_t, cmd_t) cmds;
} cmd_manager_t;

void cmd_manager_new(cmd_manager_t *cmdm)
{
    hm_new(string_slice_t, cmd_t)(&cmdm->cmds, hasher(string_slice_t),
                                  eq(string_slice_t), NULL, NULL);
}

void cmd_manager_free(cmd_manager_t *cmdm)
{
    hm_free(string_slice_t, cmd_t)(&cmdm->cmds);
}

void run_cmd_handler(cmd_t const *cmd, slice(string_slice_t) arguments)
{
    (void)cmd;
    string_slice_t const *file = arguments.data;

    drop(program_free) program_t program;
    program_new(&program, file->data);

    program_run(&program);
}

void tests_cmd_handler(cmd_t const *cmd, slice(string_slice_t) arguments)
{
    (void)cmd, (void)arguments;
    todo("implement `tests` command");
}

struct cmd_data
{
    char const *name;
    void (*handler)(cmd_t const *, slice(string_slice_t));
};

void init_cmds(cmd_manager_t *cmdm)
{
    struct cmd_data cmds[] = {{.name = "run", .handler = run_cmd_handler},
                              {.name = "tests", .handler = tests_cmd_handler}};

    for (size_t i = 0; i < (sizeof(cmds) / sizeof(cmds[0])); ++i)
    {
        struct cmd_data cmd = cmds[i];

        string_slice_t name = as_slice(cmd.name);

        hm_insert(string_slice_t, cmd_t)(&cmdm->cmds, name,
                                         (cmd_t){name, cmd.handler});
    }
}
