#ifndef ERROR_H
#define ERROR_H

#include <errno.h>
#include <string.h>

#include "utils.h"

/*
 * Print internal error
 */
#define PRINTINTERR(format, ...) \
    print_int_error##__VA_OPT__(f)(__FILE__, __LINE__, format __VA_OPT__(, ) __VA_ARGS__)

#define FUNCFAILED(f, ...) \
    PRINTINTERR(f "() failed" __VA_OPT__(": %s", __VA_ARGS__))

#define ERRCHK_PRINT_(...)        \
    do {                          \
        PRINTINTERR(__VA_ARGS__); \
    } while (0)

/*
 * If cond is true, return ERR. Also call print_error or
 * print_errorf if error message or format string is given.
 */
#define ERRCHK_RET(cond, ...)                       \
    do {                                            \
        if (cond) {                                 \
            __VA_OPT__(ERRCHK_PRINT_(__VA_ARGS__);) \
            return ERR;                             \
        }                                           \
    } while (0)

#define ERRCHK_GOTO(cond, ret, label, ...)          \
    do {                                            \
        if (cond) {                                 \
            __VA_OPT__(ERRCHK_PRINT_(__VA_ARGS__);) \
            ret = ERR;                              \
            goto label;                             \
        }                                           \
    } while (0)

#define ERRCHK_MSG_(x) "'" x "'"

#define ERRCHK_RET_MSG(cond, ...) \
    ERRCHK_RET(cond, ERRCHK_MSG_(#cond) __VA_OPT__(": %s", ) __VA_ARGS__)

#define ERRCHK_GOTO_MSG(cond, ret, label, ...) \
    ERRCHK_GOTO(cond, ret, label,              \
                ERRCHK_MSG_(#cond) __VA_OPT__(": %s", ) __VA_ARGS__)

#define ERRCHK_RET_ERN(cond) ERRCHK_RET_MSG(cond, strerror(errno))
#define ERRCHK_GOTO_ERN(cond, ret, label) ERRCHK_GOTO_MSG(cond, ret, label, strerror(errno))

/*
 * Shortcut for ERRCHK_*_RET(expr != OK)
 */
#define ERRCHK_RET_OK(e)        ERRCHK_RET((e) != OK)
#define ERRCHK_GOTO_OK(e, r, l) ERRCHK_GOTO((e) != OK, r, l)

void print_error(const char *error_msg);
void print_errorf(const char *format, ...);
void print_int_error(const char *file, unsigned long line, const char *msg);
void print_int_errorf(const char *file, unsigned long line, const char *format, ...);

#endif
