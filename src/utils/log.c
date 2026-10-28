#include <stdarg.h>
#include <stdio.h>

#include "config.h"
#include "utils/log.h"

#define base_print(f)                                                          \
    va_list args;                                                              \
    va_start(args, fmt);                                                       \
    f(fmt, args);                                                              \
    va_end(args);

void vprint(char const *fmt, va_list args) { vfprintf(stdout, fmt, args); }

void print(char const *fmt, ...) { base_print(vprint) }

void vprintln(char const *fmt, va_list args)
{
    vprint(fmt, args);
    fprintf(stdout, "\n");
}

void println(char const *fmt, ...) { base_print(vprintln) }

void veprint(char const *fmt, va_list args) { vfprintf(stderr, fmt, args); }

void eprint(char const *fmt, ...) { base_print(veprint) }

void veprintln(char const *fmt, va_list args)
{
    veprint(fmt, args);
    fprintf(stderr, "\n");
}

void eprintln(char const *fmt, ...) { base_print(veprintln) }

void vdprint(char const *fmt, va_list args)
{
    if (!CONFIG.debug)
        return;

    fprintf(stderr, "[DEBUG] ");
    vfprintf(stderr, fmt, args);
}

void dprint(char const *fmt, ...) { base_print(vdprint) }

void vdprintln(char const *fmt, va_list args)
{
    vdprint(fmt, args);
    fprintf(stderr, "\n");
}

void dprintln(char const *fmt, ...) { base_print(vdprintln) }
