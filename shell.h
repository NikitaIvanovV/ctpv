#ifndef SHELL_H
#define SHELL_H

#define SHELL_ARGS(script, ...) \
    { "sh", "-c", script, "sh", __VA_ARGS__ __VA_OPT__(,) NULL }

#endif
