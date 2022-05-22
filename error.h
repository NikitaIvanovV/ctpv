#ifndef ERROR_H
#define ERROR_H

#define _ERRCHK_RET_PR(format, ...) \
    print_error##__VA_OPT__(f)(format __VA_OPT__(, ) __VA_ARGS__)

/*
 * If cond is true, return ERR. Also call print_error or
 * print_errorf if error message or format string is given.
 */
#define ERRCHK_RET(cond, ...)                        \
    do {                                             \
        if (cond) {                                  \
            __VA_OPT__(_ERRCHK_RET_PR(__VA_ARGS__);) \
            return ERR;                              \
        }                                            \
    } while (0)

/*
 * Shortcut for ERRCHK_RET(expr != OK)
 */
#define ERRCHK_RET_OK(expr, ...) \
    ERRCHK_RET((expr) != OK __VA_OPT__(, ) __VA_ARGS__)

enum {
    OK,
    ERR,
};

void print_error(char const *error_msg);
void print_errorf(char const *format, ...);

#endif
