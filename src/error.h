#ifndef ERROR_H
#define ERROR_H

#include <errno.h>
#include <string.h>

#include "utils.h"

#define INTERRMSG "internal error: "

/*
 * Add error source to error message
 */
#define ERRSRC(msg) (__FILE__ ":" STRINGIZE(__LINE__) ": " msg)

/*
 * Print internal error
 */
#define PRINTINTERR(format, ...) \
    print_error##__VA_OPT__(f)(ERRSRC(format) __VA_OPT__(, ) __VA_ARGS__)

#define FUNCFAILED(f, ...) \
    PRINTINTERR(INTERRMSG f "() failed" __VA_OPT__(": %s", __VA_ARGS__))

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

#define ERRCHK_MSG_(x) INTERRMSG "'" x "'"

#define ERRCHK_RET_MSG(cond, ...) \
    ERRCHK_RET(cond, ERRCHK_MSG_(#cond) __VA_OPT__(": %s", ) __VA_ARGS__)

#define ERRCHK_GOTO_MSG(cond, ret, label, ...) \
    ERRCHK_GOTO(cond, ret, label,              \
                ERRCHK_MSG_(#cond) __VA_OPT__(": %s", ) __VA_ARGS__)

#define ERRCHK_RET_ERN(cond) ERRCHK_RET_MSG(cond, strerror(errno))
#define ERRCHK_GOTO_ERN(cond, ret, label) \
    ERRCHK_GOTO_MSG(cond, ret, label, strerror(errno))

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

void print_error(const char *error_msg);
void print_errorf(const char *format, ...);

#endif
