#ifndef SHELL_H
#define SHELL_H

#include "utils.h"

#define SHELL_ARGS(script, ...) \
    { "/bin/sh", "-c", script, "/bin/sh", __VA_ARGS__ __VA_OPT__(,) NULL }

RESULT run_script(char *script, size_t script_len, int *exitcode, int *signal,
                  SpawnProg sp, void *sp_arg);

#endif
