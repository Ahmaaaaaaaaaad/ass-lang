#include "core/collection/hash_map.h"
#include "core/collection/slice.h"
#include "core/collection/vec.h"
#include "program/program.h"
#include "utils/control.h"
#include "utils/drop.h"
#include "utils/guard.h"

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

// vector_t files_from(const char *dirpath)
// {
//     vector_t path_vec = str_vec_with_cap(8);

// #ifdef _WIN32
//     WIN32_FIND_DATA ff_data;
//     char spath[MAX_PATH];

//     snprintf(spath, MAX_PATH, "%s\\*", dirpath);

//     HANDLE h_find = FindFirstFile(spath, &ff_data);
//     if (h_find == INVALID_HANDLE_VALUE)
//         terminate("[files_from] failed to open directory", EXIT_FAILURE);

//     do
//     {
//         if (strcmp(ff_data.cFileName, ".") == 0 ||
//             strcmp(ff_data.cFileName, "..") == 0)
//             continue;

//         char cpath[PATH_MAX];
//         snprintf(cpath, PATH_MAX, "%s\\%s", dirpath, ff_data.cFileName);

//         struct stat buf;
//         int status = stat(cpath, &buf);

//         if (S_ISDIR(buf.st_mode))
//         {
//             clean(vec_free) vector_t files = files_from(cpath);
//             vec_insert_full_vec(&path_vec, &files, path_vec.size, true);

//             continue;
//         }

//         *str_vec_use(&path_vec) = string_from(cpath);

//     } while (FindNextFile(h_find, &ff_data) != 0);

//     FindClose(h_find);

// #else
//     DIR *dir;
//     struct dirent *entry;

//     dir = opendir(dirpath);
//     if (dir == NULL)
//         terminate("[files_from] failed to open directory", EXIT_FAILURE);

//     while ((entry = readdir(dir)) != NULL)
//     {
//         if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") ==
//         0)
//             continue;

//         char cpath[PATH_MAX];
//         snprintf(cpath, PATH_MAX, "%s/%s", dirpath, entry->d_name);

//         struct stat buf;
//         int status = stat(cpath, &buf);

//         if (S_ISDIR(buf.st_mode))
//         {
//             // Does not require automatic clean-up as the full vector is
//             moved clean(vec_free) vector_t files = files_from(cpath);
//             vec_insert_full_vec(&path_vec, &files, path_vec.size, true);

//             continue;
//         }

//         *str_vec_use(&path_vec) = string_from(cpath);
//     }

//     closedir(dir);

// #endif

//     return path_vec;
// }

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

int main(int argc, char **argv)
{
    if (argc < 2)
        todo("display help information");

    string_slice_t command = as_slice(argv[1]);

    drop(vec_free(string_slice_t)) vec(string_slice_t) arguments;
    vec_new(string_slice_t)(&arguments, NULL, (size_t)argc);
    for (size_t i = 2; i < (size_t)argc; ++i)
    {
        *vec_expand(string_slice_t)(&arguments) =
            (string_slice_t){.data = argv[i], .n = strlen(argv[i])};
    }

    drop(cmd_manager_free) cmd_manager_t cmdm;
    cmd_manager_new(&cmdm);
    init_cmds(&cmdm);

    cmd_t const *cmd = hm_get(string_slice_t, cmd_t)(&cmdm.cmds, &command);
    if (cmd == NULL)
        todo("throw error: unknown command '" slice_fmt "'.", use(command));

    cmd->handler(cmd, view(string_slice_t)(&arguments, 0, arguments.len));

    return 0;
}

// int main(int argc, char **argv)
// {
//     for (size_t i = 1; i < (size_t)argc; ++i)
//         println("arg[%zu]: %s", i, argv[i]);

//     char const *fpath = "tests/lexical/keywords.ass";

//     drop(program_free) program_t program;
//     program_new(&program, fpath);

//     program_run(&program);

//     return 0;
// }
