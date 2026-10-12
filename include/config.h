#ifndef __CONFIG_H__
#define __CONFIG_H__

/// Main internal configuration file of ASS-lang

#include <stdbool.h>
#include <stdint.h>

typedef enum : uint8_t
{
    OCONF_GEN_SYM_TABLE = 1 << 0,
    OCONF_LOG_SYM_TABLE = 1 << 1,
} out_config_t;

typedef struct
{
    char const *file_ext;
    out_config_t out_conf;
    bool debug;
} config_t;

extern config_t CONFIG;

#endif
