#include <stdio.h>

#include "error.h"
#include "utils.h"

int err_internal_error = 0;

void print_error(const char *error_msg)
{
    /* We print errors to stdout because lf file manager
     * doesn't print stderr in the preview window. */
    fprintf(stdout, "%s: %s\n", program, error_msg);
    fflush(stdout);
}

void print_errorf(const char *format, ...)
{
    char s[1024];
    FORMATTED_STRING(s, format);
    print_error(s);
}
