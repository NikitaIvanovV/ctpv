#include <stdio.h>

#include "error.h"
#include "utils.h"

void print_error(char const *error_msg)
{
    fprintf(stderr, "%s: %s\n", program, error_msg);
}

void print_errorf(char const *format, ...)
{
    char s[1024];
    FORMATTED_STRING(s, format);
    print_error(s);
}
