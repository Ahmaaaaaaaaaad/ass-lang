#ifndef __UTILS_GUARD_H__
#define __UTILS_GUARD_H__

#include "utils/control.h"

#define assert(expr, msg_fmt, ...)                                             \
    {                                                                          \
        if (!(expr))                                                           \
            panic(msg_fmt __VA_OPT__(, ) __VA_ARGS__);                         \
    }

#define todo(fmt, ...)                                                         \
    {                                                                          \
        panic("not yet implemented: " fmt __VA_OPT__(, ) __VA_ARGS__);         \
    }

#endif
