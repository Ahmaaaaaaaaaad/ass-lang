#include <stdlib.h>
#include <string.h>

#include "core/collection/hash_map.h"
#include "core/collection/slice.h"
#include "core/string/slice.h"
#include "program/program.h"
#include "utils/drop.h"
#include "utils/guard.h"
#include "utils/log.h"

typedef struct
{
    string_slice_t name, sh;
} cmd_opt_t;

typedef struct cmd_t
{
    string_slice_t name;
    int (*handler)(struct cmd_t const *cmd, slice(string_slice_t) arguments);
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

int run_cmd_handler(cmd_t const *cmd, slice(string_slice_t) arguments)
{
    (void)cmd;
    if (arguments.n < 1)
    {
        eprintln("error: missing file to run, usage: ass run <file>");
        return EXIT_FAILURE;
    }

    string_slice_t const *file = arguments.data;

    drop(program_free) program_t program;
    program_new(&program, file->data);

    return program_run(&program) ? EXIT_SUCCESS : EXIT_FAILURE;
}

int tests_cmd_handler(cmd_t const *cmd, slice(string_slice_t) arguments)
{
    (void)cmd, (void)arguments;
    todo("implement `tests` command");
}

struct cmd_data
{
    char const *name;
    int (*handler)(cmd_t const *, slice(string_slice_t));
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

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        eprintln("usage: ass <command> [arguments]\n\n"
                 "commands:\n"
                 "  run <file>  tokenize an ASS-lang source file");
        return EXIT_FAILURE;
    }

    string_slice_t command = as_slice(argv[1]);

    drop(vec_free(string_slice_t)) vec(string_slice_t) arguments;
    vec_new(string_slice_t)(&arguments, NULL, (size_t)argc);
    for (size_t i = 2; i < (size_t)argc; ++i)
        *vec_expand(string_slice_t)(&arguments) = as_slice(argv[i]);

    drop(cmd_manager_free) cmd_manager_t cmdm;
    cmd_manager_new(&cmdm);

    init_cmds(&cmdm);

    cmd_t const *cmd = hm_get(string_slice_t, cmd_t)(&cmdm.cmds, &command);
    if (cmd == NULL)
    {
        eprintln("error: unknown command '" slice_fmt "'", use(command));
        return EXIT_FAILURE;
    }

    return cmd->handler(cmd, view(string_slice_t)(&arguments, 0, arguments.len));
}
