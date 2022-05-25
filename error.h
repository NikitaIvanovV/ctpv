#ifndef ERROR_H
#define ERROR_H

#include <errno.h>
#include <string.h>

#include "utils.h"

#define ERRNOS strerror(errno)

#define FUNCFAILED(f) f "() failed: %s"

/*
 * Print internal error
 */
#define PRINTINTERR(format, ...) \
    print_error##__VA_OPT__(f)(ERRS(format) __VA_OPT__(, ) __VA_ARGS__)

/*
 * Add error source to error message
 */
#define ERRS(msg) (__FILE__ ":" STRINGIZE(__LINE__) ": " msg)

/*
 * If cond is true, return ERR. Also call print_error or
 * print_errorf if error message or format string is given.
 */
#define ERRCHK_RET(cond, ...)                     \
    do {                                          \
        if (cond) {                               \
            __VA_OPT__(PRINTINTERR(__VA_ARGS__);) \
            return ERR;                           \
        }                                         \
    } while (0)

#define ERRCHK_GOTO(cond, ret, label, ...)        \
    do {                                          \
        if (cond) {                               \
            __VA_OPT__(PRINTINTERR(__VA_ARGS__);) \
            ret = ERR;                            \
            goto label;                           \
        }                                         \
    } while (0)

/*
 * Shortcut for ERRCHK_RET(expr != OK)
 */
#define ERRCHK_RET_OK(expr, ...) \
    ERRCHK_RET((expr) != OK __VA_OPT__(, ) __VA_ARGS__)

#define ERRCHK_GOTO_OK(expr, ...) \
    ERRCHK_GOTO((expr) != OK __VA_OPT__(, ) __VA_ARGS__)

enum {
    OK,
    ERR,
};

void print_error(char const *error_msg);
void print_errorf(char const *format, ...);

#endif
