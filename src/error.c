#include <stdio.h>

#include "error.h"
#include "utils.h"

void print_error(const char *error_msg)
{
    /* We print errors to stdout because lf file manager
     * doesn't print stderr in the preview window. */
    fprintf(stdout, "%s: %s\n", program, error_msg);
    fflush(stdout);
}

void print_errorf(const char *format, ...)
{
    char s[512];
    FORMATTED_STRING(s, format);

    print_error(s);
}

void print_int_error(const char *file, unsigned long line, const char *msg)
{
    print_errorf("%s:%lu: internal error: %s", file, line, msg);
}

void print_int_errorf(const char *file, unsigned long line, const char *format, ...)
{
    char s[512];
    FORMATTED_STRING(s, format);

    print_int_error(file, line, s);
}
