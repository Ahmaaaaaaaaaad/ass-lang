#ifndef __UTILS_LOG_H__
#define __UTILS_LOG_H__

#include <stdarg.h>

void vprint(char const *fmt, va_list args);
void print(char const *fmt, ...);
void vprintln(char const *fmt, va_list args);
void println(char const *fmt, ...);

void veprint(char const *fmt, va_list args);
void eprint(char const *fmt, ...);
void veprintln(char const *fmt, va_list args);
void eprintln(char const *fmt, ...);

void vdprint(char const *fmt, va_list args);
void dprint(char const *fmt, ...);
void vdprintln(char const *fmt, va_list args);
void dprintln(char const *fmt, ...);

#endif
