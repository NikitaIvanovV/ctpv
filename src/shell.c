#include <string.h>

#include "shell.h"
#include "error.h"
#include "../gen/helpers.h"

/*
 * Returns string with helpers.sh prepended
 *
 * User must call free()
 */
char *prepend_helpers(char *str, size_t len)
{
    char *buf, *b;
    size_t l, helpers_len = LEN(scr_helpers_sh) - 1;

    if (!(buf = malloc(sizeof(*buf) * (helpers_len + len)))) {
        PRINTINTERR(FUNCFAILED("malloc"), ERRNOS);
        abort();
    }

    b = buf;
    l = helpers_len;
    memcpy(b, scr_helpers_sh, l * sizeof(*b));

    b += l * sizeof(*str);
    l = len;
    memcpy(b, str, l * sizeof(*b));

    return buf;
}
