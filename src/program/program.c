#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "automaton/state.h"
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

static void canonicalize_path(string_t *abs_path, const char *path)
{
#ifdef _WIN32
    DWORD size = GetFullPathNameA(path, 0, NULL, NULL);
    if (size == 0)
        return NULL;

    vec_update_cap(char)(abs_path, size);
    if (GetFullPathNameA(path, size, resolved, NULL) == 0)
        todo("panic: cannot find absolute path of '%s'", path);

    for (char *p = resolved; *p; p++)
        if (*p == '\\')
            *p = '/';

#else
    vec_update_cap(char)(abs_path, PATH_MAX);
    if (realpath(path, abs_path->data) == NULL)
        todo("panic: cannot find absolute path of '%s'", path);

#endif
}

void program_new(program_t *program, char const *fpath)
{
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

    size_t fext_len = strlen(CONFIG.file_ext);
    if (program->file_path.len >= fext_len &&
        string_ends_with(&program->file_path, CONFIG.file_ext))
    {
        panic("entry point file provided must be an ASS-lang file and ends "
              "with `.ass`");
    }

    canonicalize_path(&program->file_path, program->file_path.data);

    FILE *file = fopen(program->file_path.data, "r");
    if (file == NULL)
        panic("failed to read file: %s", program->file_path.data);

    fseek(file, 0, SEEK_END);
    size_t fsize = ftell(file);
    fseek(file, 0, 0);

    string_init_wcap(&program->src, fsize);
    size_t read_size = fread(program->src.data, sizeof(char), fsize, file);

    if (fsize != read_size)
        panic("failed to completely read file: %s", program->file_path.data);

    program->src.len = read_size;

    fclose(file);
}

void program_run(program_t *program)
{
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    program_read_source(program);
    dprintln("running file '%s'", program->file_path.data);

    drop(lexer_free) lexer_t lexer;
    lexer_new(&lexer, view(char)(&program->src, 0, program->src.len));
    lexer_analyze(&lexer);

    clock_gettime(CLOCK_MONOTONIC, &end);
    long elapsed_ns = (end.tv_nsec - start.tv_nsec),
         elapsed_mus = elapsed_ns / 1000;
    dprintln("time elapsed: %ld ns, %ld μs", elapsed_ns, elapsed_mus);
}
