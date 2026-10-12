#include "config.h"

config_t CONFIG = (config_t){
    .file_ext = ".ass",
    .out_conf = OCONF_GEN_SYM_TABLE /// Generates the symbol table file per file
                                    // | OCONF_LOG_SYM_TABLE ///  Logs the
                                    // symbol table per file in the stdout
    ,
    .debug = true,
};
