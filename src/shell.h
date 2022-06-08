#ifndef SHELL_H
#define SHELL_H

#include "utils.h"

#define SHELL_ARGS(script, ...) \
    { "/bin/sh", "-c", script, "/bin/sh", __VA_ARGS__ __VA_OPT__(,) NULL }

char *prepend_helpers(char *str, size_t len);

#endif
