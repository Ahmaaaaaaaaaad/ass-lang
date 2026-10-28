#ifndef __PROGRAM_PROGRAM_H__
#define __PROGRAM_PROGRAM_H__

#include "core/string/slice.h"
#include "core/string/string.h"

typedef struct
{
    string_t src, file_path;
} program_t;

void program_new(program_t *program, char const *fpath);
void program_free(program_t *program);

void program_read_source(program_t *program);
/// Returns `false` if the program had errors.
bool program_run(program_t *program);

#endif
