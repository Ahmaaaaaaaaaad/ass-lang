#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "config.h"
#include "core/string/slice.h"
#include "program/program.h"
#include "utils/control.h"
#include "utils/guard.h"

#ifdef _WIN32
#include <windows.h>
#define PATH_SEPARATOR '\\'
#else
#include <linux/limits.h>
#include <unistd.h>
#define PATH_SEPARATOR '/'
#endif

static bool is_regular_file(const char *path)
{
    struct stat buf;
    int status = stat(path, &buf);

    if (status != 0)
        return false;

    return S_ISREG(buf.st_mode);
}

static char *canonicalize_path(const char *path)
{
    char *resolved = NULL;

#ifdef _WIN32
    DWORD size = GetFullPathNameA(path, 0, NULL, NULL);
    if (size == 0)
        return NULL;

    resolved = malloc(size * sizeof(char));
    if (resolved == NULL)
        return NULL;

    if (GetFullPathNameA(path, size, resolved, NULL) == 0)
    {
        free(resolved);
        return NULL;
    }

    for (char *p = resolved; *p; p++)
        if (*p == '\\')
            *p = '/';

#else
    resolved = malloc(PATH_MAX * sizeof(char));
    if (resolved == NULL)
        return NULL;

    if (realpath(path, resolved) == NULL)
    {
        free(resolved);
        return NULL;
    }

#endif

    return resolved;
}

void program_new(program_t *program, char const *fpath)
{
    program->file_path = as_slice(fpath);
}

void program_free(program_t *program) { string_free(&program->src); }

void program_read_source(program_t *program)
{
    if (!is_regular_file(program->file_path.data))
        panic("invalid file path provided");

    size_t fext_len = strlen(CONFIG.file_ext);
    if (program->file_path.n >= fext_len &&
        string_slice_ends_with(&program->file_path, CONFIG.file_ext))
    {
        panic("entry point file provided must be an ASS-lang file and ends "
              "with `.ass`");
    }

    program->file_path = as_slice(canonicalize_path(program->file_path.data));

    FILE *file = fopen(program->file_path.data, "r");
    if (file == NULL)
        panic("failed to read file: " slice_fmt, use(program->file_path));

    fseek(file, 0, SEEK_END);
    size_t fsize = ftell(file);
    fseek(file, 0, 0);

    string_init_wcap(&program->src, fsize);
    size_t read_size = fread(program->src.data, sizeof(char), fsize, file);

    if (fsize != read_size)
        panic("failed to completely read file: " slice_fmt,
              use(program->file_path));

    fclose(file);
}

void program_run(program_t *program)
{
    program_read_source(program);
    dprintln("running file '" slice_fmt "'", use(program->file_path));

    todo("lexical analyzer");

    todo("syntax analyzer");
}
