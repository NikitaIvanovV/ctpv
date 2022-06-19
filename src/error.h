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

#ifndef NO_TRACEBACK
#define ERRCHK_PRINT_(...)                                                  \
    do {                                                                    \
        int __a = 0;                                                        \
        __VA_OPT__(PRINTINTERR(__VA_ARGS__); __a = err_internal_error = 1;) \
        if (!__a && err_internal_error)                                     \
            PRINTINTERR();                                                  \
    } while (0)
#endif
#ifdef NO_TRACEBACK
#define ERRCHK_PRINT_(...)                    \
    do {                                      \
        __VA_OPT__(PRINTINTERR(__VA_ARGS__);) \
    } while (0)
#endif

/*
 * If cond is true, return ERR. Also call print_error or
 * print_errorf if error message or format string is given.
 */
#define ERRCHK_RET(cond, ...)           \
    do {                                \
        if (cond) {                     \
            ERRCHK_PRINT_(__VA_ARGS__); \
            return ERR;                 \
        }                               \
    } while (0)

#define ERRCHK_GOTO(cond, ret, label, ...) \
    do {                                   \
        if (cond) {                        \
            ERRCHK_PRINT_(__VA_ARGS__);    \
            ret = ERR;                     \
            goto label;                    \
        }                                  \
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
 * Shortcut for ERRCHK_*_RET(expr != OK)
 */
#define ERRCHK_RET_OK(e)        ERRCHK_RET((e) != OK)
#define ERRCHK_GOTO_OK(e, r, l) ERRCHK_GOTO((e) != OK, r, l)

extern int err_internal_error;

enum {
    OK,
    ERR,
};

void print_error(const char *error_msg);
void print_errorf(const char *format, ...);

#endif
