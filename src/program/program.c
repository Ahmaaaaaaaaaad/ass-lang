#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "config.h"
#include "core/string/slice.h"
#include "core/string/string.h"
#include "lexical/analyzer.h"
#include "program/program.h"
#include "utils/control.h"
#include "utils/drop.h"
#include "utils/guard.h"

#ifdef _WIN32
#include <windows.h>
#define PATH_SEPARATOR '\\'
#else
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

static void canonicalize_path(string_t *abs_path, const char *path)
{
#ifdef _WIN32
    DWORD size = GetFullPathNameA(path, 0, NULL, NULL);
    if (size == 0)
        panic("cannot find absolute path of '%s'", path);

    drop(string_free) string_t resolved;
    string_init_wcap(&resolved, size);

    DWORD len = GetFullPathNameA(path, size, resolved.data, NULL);
    if (len == 0 || len >= size)
        panic("cannot find absolute path of '%s'", path);

    for (char *p = resolved.data; *p; p++)
        if (*p == '\\')
            *p = '/';

    string_assign(abs_path, resolved.data);
#else
    char resolved[PATH_MAX];
    if (realpath(path, resolved) == NULL)
        panic("cannot find absolute path of '%s'", path);

    string_assign(abs_path, resolved);
#endif
}

void program_new(program_t *program, char const *fpath)
{
    string_init(&program->src);
    string_from(&program->file_path, fpath);
}

void program_free(program_t *program)
{
    string_free(&program->src);
    string_free(&program->file_path);
}

void program_read_source(program_t *program)
{
    if (!is_regular_file(program->file_path.data))
        panic("invalid file path provided");

    if (!string_ends_with(&program->file_path, CONFIG.file_ext))
    {
        panic("entry point file provided must be an ASS-lang file and ends "
              "with `%s`",
              CONFIG.file_ext);
    }

    canonicalize_path(&program->file_path, program->file_path.data);

    // Binary mode, so that `\r\n` is kept as-is and the size matches `ftell`.
    FILE *file = fopen(program->file_path.data, "rb");
    if (file == NULL)
        panic("failed to read file: %s", program->file_path.data);

    fseek(file, 0, SEEK_END);
    size_t fsize = ftell(file);
    fseek(file, 0, SEEK_SET);

    vec_update_cap(char)(&program->src, fsize + 1);
    size_t read_size = fread(program->src.data, sizeof(char), fsize, file);
    fclose(file);

    if (fsize != read_size)
        panic("failed to completely read file: %s", program->file_path.data);

    program->src.len = read_size;
    program->src.data[read_size] = '\0';
}

bool program_run(program_t *program)
{
    program_read_source(program);
    dprintln("running file '%s'", program->file_path.data);

    drop(lexer_free) lexer_t lexer;
    lexer_new(&lexer, program->file_path.data,
              as_slicen(program->src.data, program->src.len));
    lexer_analyze(&lexer);

    lexer_print_tokens(&lexer);

    return lexer.err_count == 0;
}
